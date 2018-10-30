#include "main.hpp"
#include "krawlers.hpp"
#include <boost/mpi/environment.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

template<typename T>
void atomic_print(std::vector<T> vec) {
    std::ostringstream oss;

    for(auto &e: vec)
        oss << e << std::endl;
    
    std::cout << oss.str() << std::endl << std::flush;
}

void get_env(envvars* env) {
    const char * ENV_nodes = std::getenv("NODES");
    const char * ENV_url = std::getenv("URL");

    env->NODES = ENV_nodes == NULL ? 4 : std::stoi(ENV_nodes);

    if(ENV_url == NULL)
        throw std::invalid_argument("Environment variable URL must be set. Exiting...");
    else
        env->URL = ENV_url;
}

// Based on: https://stackoverflow.com/a/37708514/4922572
std::vector<std::vector<std::string>> split_vector(const std::vector<std::string>& vec, size_t n) {
    std::vector<std::vector<std::string>> outVec;

    int length = vec.size() / n;
    int remain = vec.size() % n;

    int begin = 0;
    int end = 0;

    for (unsigned int i = 0; i < std::min(n, vec.size()); ++i) {
        end += (remain > 0) ? (length + !!(remain--)) : length;

        outVec.push_back(
            std::vector<std::string>(vec.begin() + begin, vec.begin() + end)
        );

        begin = end;
    }

    return outVec;
}

int main(int argc, char **argv) {

    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;

    std::vector<std::vector<std::string>> all_pages_splitted_by_process;
    KrawlerS ks;

    if(world.rank() == 0) {
        envvars * env = new envvars;

        try {
            get_env(env);
        }
        catch(const std::invalid_argument &e) {
            std::cout << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        std::vector<std::string> pages = ks.get_pages(env->URL);
        /**
         * pages = [url_1, url_2, url_3, ...]
         **/

        all_pages_splitted_by_process = split_vector(pages, world.size());
        /**
         * all_pages_splitted_by_process = [
         *     [url_01, url_02, url_03, ...],
         *     [url_11, url_12, url_13, ...],
         *     [url_21, url_22, url_23, ...],
         *     [url_31, url_32, url_33, ...],
         *     ...
         * ]
         **/
    }

    std::vector<std::string> process_specific_pages;
    boost::mpi::scatter(
        world,
        all_pages_splitted_by_process,
        process_specific_pages,
        0
    );

    atomic_print(process_specific_pages);

    std::vector<std::ostringstream> products = ks.crawl(process_specific_pages);

    for(auto &p: products)
        std::cout << p.str() << std::endl;

    return EXIT_SUCCESS;
}
