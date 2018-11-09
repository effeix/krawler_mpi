#ifndef __KRAWLERS_H__
#define __KRAWLERS_H__

#include "product.hpp"

#include <chrono>
#include <string>
#include <vector>

#include <boost/regex.hpp>

typedef std::chrono::high_resolution_clock Time;

class KrawlerS {
public:
    KrawlerS();
    std::vector<std::string> crawl(
        std::vector<std::string> urls, double& total_idle_time);
    std::vector<std::string> get_pages(std::string url);
    std::vector<std::string> product_info(std::string product_url);
    std::string product_category(std::string product_url);
    std::string search(std::string& page_content, std::string& expr);
    std::vector<std::string> search_many(
        std::string& page_content,
        std::string& expr
    );
    Product new_product(
        std::string& link,
        double& product_download
    );

    std::string re_last_page;
    std::string re_product_name;
    std::string re_product_description;
    std::string re_product_img_url;
    std::string re_product_price;
    std::string re_product_installment_qty;
    std::string re_product_price_in_installment;
    std::string re_product_category;
    std::string re_product_link;
};

#endif/*__KRAWLERS_H__*/
