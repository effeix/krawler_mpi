#include "krawlers.hpp"

#include "curlhandler.h"
#include "product.hpp"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
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

Product KrawlerS::new_product(std::string& link, double& download_time) {

    Time::time_point t0, t1, t2, t3;
    double elapsed_analysis;

    t0 = Time::now();

        t1 = Time::now();
            std::string product_page = http_get(link);
        t2 = Time::now();

        std::string name = search(product_page, re_product_name);
        std::string description = search(product_page, re_product_description);
        std::string pic_url = search(product_page, re_product_img_url);
        std::string price = search(product_page, re_product_price);
        std::string installment_qty = search(product_page,
            re_product_installment_qty);
        std::string price_in_installment = search(product_page,
            re_product_price_in_installment);
    
    t3 = Time::now();

    download_time = duration_cast<duration<double>>(t2 - t1).count();

    elapsed_analysis = duration_cast<duration<double>>(t3 - t0).count();
    std::cerr << elapsed_analysis << std::endl;

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
    std::vector<std::string> urls,
    double& process_idle_time) {

    std::vector<std::string> all_products;

    std::string category = product_category(urls[0]);

    Time::time_point t0, t1;
    double download_time;

    for(unsigned int i = 0; i < urls.size(); i++) {
        t0 = Time::now();
            std::string products_page = http_get(urls[i]);
        t1 = Time::now();
        process_idle_time += duration_cast<duration<double>>(t1 - t0).count();

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

    return all_products;
}
