#include "main.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/mpi/environment.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>

#include "krawlers.hpp"

using namespace std::chrono;

void get_env(envvars* env) {
    const char * ENV_nodes = std::getenv("NODES");
    const char * ENV_url = std::getenv("URL");

    env->NODES = ENV_nodes == NULL ? 4 : std::stoi(ENV_nodes);

    if(ENV_url == NULL)
        throw std::invalid_argument(
            "Environment variable URL must be set. Exiting...");
    else
        env->URL = ENV_url;
}

// Based on: https://stackoverflow.com/a/37708514/4922572
std::vector<std::vector<std::string>> split_vector(
    const std::vector<std::string>& vec, size_t n) {
    std::vector<std::vector<std::string>> splitted;

    int length = vec.size() / n;
    int remain = vec.size() % n;

    int begin = 0;
    int end = 0;

    for (unsigned int i = 0; i < std::min(n, vec.size()); ++i) {
        // Remaining elements will be distributed one by one across chunks
        end += (remain > 0) ? (length + !!(remain--)) : length;

        splitted.push_back(
            std::vector<std::string>(vec.begin() + begin, vec.begin() + end)
        );

        begin = end;
    }

    return splitted;
}

int main(int argc, char **argv) {

    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    std::vector<std::vector<std::string>> all_pages_splitted_by_process;
    KrawlerS ks;

    Time::time_point total_program_time_start;
    Time::time_point total_program_time_end;

    if(world.rank() == ROOT_NODE) {
        total_program_time_start = Time::now();

        envvars * env = new envvars;

        try {
            get_env(env);
        }
        catch(const std::invalid_argument &e) {
            std::cout << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        std::vector<std::string> pages = ks.get_pages(env->URL);
        all_pages_splitted_by_process = split_vector(pages, world.size());
    }

    std::vector<std::string> process_specific_pages;
    boost::mpi::scatter(
        world,
        all_pages_splitted_by_process,
        process_specific_pages,
        ROOT_NODE
    );

    double process_idle_time = 0;
    double total_idle_time = 0;
    std::vector<std::string> process_specific_products = ks.crawl(
        process_specific_pages,
        process_idle_time 
    );

    boost::mpi::reduce(
        world,
        process_idle_time,
        total_idle_time,
        std::plus<double>(),
        ROOT_NODE
    );

    std::vector<std::vector<std::string>> all_products_by_process;
    boost::mpi::gather(
        world,
        process_specific_products,
        all_products_by_process,
        ROOT_NODE
    );

    if(world.rank() == ROOT_NODE) {
        unsigned int total_products = 0;
        unsigned int length_all_products = all_products_by_process.size();
        unsigned int product_qty = 0;

        for(unsigned int i = 0; i < length_all_products; i++) {
            for(std::string& product: all_products_by_process[i]) {
                std::cout << product << std::endl;
            }
        }

        for(unsigned int i = 0; i < length_all_products; i++) {
            product_qty = all_products_by_process[i].size();
            total_products += product_qty;

            std::cout << i << ": processed " << product_qty << " products";
            std::cout << std::endl;
        }

        total_program_time_end = Time::now();
        duration<double> elapsed_total = duration_cast<duration<double>>(total_program_time_end - total_program_time_start);

        std::cerr << "TOTAL_PROD_COUNT: " << total_products << std::endl;
        std::cerr << "TOTAL_IDLE_TIME: " << total_idle_time << std::endl;
        std::cerr << "TOTAL_EXEC_TIME: " << elapsed_total.count() << std::endl;
        std::cerr << "AVG_TIME_PER_PRODUCT: " << elapsed_total.count() / total_products << std::endl;
    }

    return EXIT_SUCCESS;
}
