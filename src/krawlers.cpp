#include "krawlers.hpp"

#include "curlhandler.h"
#include "product.hpp"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/optional/optional.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/regex.hpp>

using namespace std::chrono;

KrawlerS::KrawlerS() {
    re_last_page = "(?<=\"lastPage\":)(\\d+)";
    re_product_name = "(?<=\"fullTitle\":\\s)(.*?)(?=\",)";
    re_product_description = "(?<=<p class=\"description__text\"></p>)(.*?)(?=<p class=\"description__text\"></p>)";
    re_product_img_url = "(?<=showcase-product__big-img js-showcase-big-img\"\\ssrc=\")(.*?)(?=\" item)";
    re_product_price = "(?<=\"priceTemplate\":\\s\")(.*?)(?=\",)";
    re_product_installment_qty = "(?<=\"installmentQuantity\":\\s\")(.*?)(?=\",)";
    re_product_price_in_installment = "(?<=\"priceTemplate\":\\s\")(.*?)(?=\",)";
    re_product_category = "";
    re_product_link = "(?<=linkToProduct\"\\shref=\")(.*?)(?=\")";
}

std::string KrawlerS::search(std::string& page_content, std::string& expr) {
    boost::regex expression(expr);
    boost::smatch matches;

    if(boost::regex_search(page_content, matches, expression)) {
        return matches[1];
    }

    return "N/A";
}

std::vector<std::string> KrawlerS::search_many(std::string& page_content,
        std::string& expr) {
    boost::regex expression(expr);

    std::vector<std::string> all_matches;

    boost::sregex_token_iterator iter(
        page_content.begin(),
        page_content.end(),
        expression,
        0
    );
    boost::sregex_token_iterator end;

    for(; iter != end; ++iter)
        all_matches.push_back(*iter);

    return all_matches;
}

std::string KrawlerS::product_category(std::string product_url) {
    std::vector<std::string> strs;
    boost::split(strs, product_url, boost::is_any_of("/"));

    return strs[3];
}

Product KrawlerS::new_product(
    std::string& link,
    double& download_time) {

    duration<double> elapsed;

    Time::time_point product_analysis_start = Time::now();

    Time::time_point product_download_start = Time::now();
    std::string product_page = http_get(link);
    Time::time_point product_download_end = Time::now();

    std::string name = search(product_page, re_product_name);
    std::string description = search(product_page, re_product_description);
    std::string pic_url = search(product_page, re_product_img_url);
    std::string price = search(product_page, re_product_price);
    std::string installment_qty = search(product_page,
        re_product_installment_qty);
    std::string price_in_installment = search(product_page,
        re_product_price_in_installment);
    
    Time::time_point product_analysis_end = Time::now();

    elapsed = duration_cast<duration<double>>(product_download_end - product_download_start);
    download_time = elapsed.count();

    elapsed = duration_cast<duration<double>>(product_analysis_end - product_analysis_start);

    std::cerr << "PROD_TIME: " << elapsed.count() << std::endl;

    return Product(
        name,
        description,
        pic_url,
        price,
        price_in_installment,
        installment_qty,
        "",
        link
    );
}

std::vector<std::string> KrawlerS::crawl(
    std::vector<std::string> urls, double& process_idle_time) {

    std::vector<std::string> all_products;

    std::string category = product_category(urls[0]);

    double download_time;
    duration<double> elapsed;

    for(unsigned int i = 0; i < urls.size(); i++) {
        Time::time_point download_time_start = Time::now();
        std::string products_page = http_get(urls[i]);
        Time::time_point download_time_end = Time::now();
        elapsed = duration_cast<duration<double>>(download_time_end - download_time_start);
        process_idle_time += elapsed.count();

        std::vector<std::string> product_links = search_many(
            products_page,
            re_product_link
        );

        for(std::string& link: product_links) {
            Product p = new_product(link, download_time);
            p.category = category;
            all_products.push_back(p.display());

            process_idle_time += download_time;
        }
    }

    // std::ostringstream oss;
    // for(std::string& p: all_products)
    //     oss << p << std::endl;

    // std::cout << oss.str();

    return all_products;
}

std::vector<std::string> KrawlerS::get_pages(std::string url) {
    std::string first_page = http_get(url);
    std::string n_pages = search(first_page, re_last_page);
    std::string pagination = "?page=";

    std::vector<std::string> pages;

    for(int i = 1; i <= std::stoi(n_pages); i++) {
        pages.push_back(
            url + pagination + std::to_string(i)
        );
    }

    return pages;
}
