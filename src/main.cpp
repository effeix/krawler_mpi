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
    const char * ENV_url = std::getenv("URL");

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

    KrawlerS ks;

    Time::time_point t0, t1, t2, t3;
    double process_idle_time = 0;
    double total_idle_time = 0;
    double process_total_time = 0;
    std::vector<std::vector<std::string>> page_list_per_process;
    std::vector<std::string> process_specific_pages;
    std::vector<std::string> process_specific_products;
    std::vector<std::vector<std::string>> product_list_per_process;
    std::vector<double> process_times;

    if(world.rank() == ROOT_NODE) {
        envvars * env = new envvars;

        try {
            get_env(env);
        }
        catch(const std::invalid_argument &e) {
            std::cout << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        std::vector<std::string> pages = ks.get_pages(env->URL);
        page_list_per_process = split_vector(pages, world.size());
    }

    t0 = Time::now();

        boost::mpi::scatter(
            world,
            page_list_per_process,
            process_specific_pages,
            ROOT_NODE
        );

        process_specific_products = ks.crawl(
            process_specific_pages,
            process_idle_time 
        );

        boost::mpi::gather(
            world,
            process_specific_products,
            product_list_per_process,
            ROOT_NODE
        );

    t1 = Time::now();
    process_total_time = duration_cast<duration<double>>(t1 - t0).count();

    boost::mpi::reduce(
        world,
        process_idle_time,
        total_idle_time,
        std::plus<double>(),
        ROOT_NODE
    );

    boost::mpi::gather(
        world,
        process_total_time,
        process_times,
        ROOT_NODE
    );

    if(world.rank() == ROOT_NODE) {
        unsigned int total_products = 0;
        unsigned int length_all_products = product_list_per_process.size();
        unsigned int product_qty = 0;
        double program_time = *std::max_element(
            process_times.begin(), process_times.end());

        for(unsigned int i = 0; i < length_all_products; i++) {
            for(std::string& product: product_list_per_process[i]) {
                std::cout << product << std::endl;
            }
        }

        for(unsigned int i = 0; i < length_all_products; i++) {
            product_qty = product_list_per_process[i].size();
            total_products += product_qty;
        }

        std::cerr << "TOTAL_PROD_COUNT: " << total_products << std::endl;
        std::cerr << "AVG_IDLE_TIME: " << total_idle_time / world.size() << std::endl;
        std::cerr << "TOTAL_IDLE_TIME: " << total_idle_time << std::endl;
        std::cerr << "TOTAL_EXEC_TIME: " << program_time << std::endl;
        std::cerr << "AVG_TIME_PER_PRODUCT: " << program_time / total_products;
        std::cerr << std::endl;
    }

    return EXIT_SUCCESS;
}
