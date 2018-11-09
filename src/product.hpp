#ifndef __PRODUCT_H__
#define __PRODUCT_H__

#include <sstream>
#include <string>

class Product {
public:
    Product(std::string name,
            std::string description,
            std::string pic_url,
            std::string price,
            std::string price_in_installment,
            std::string installment_qty,
            std::string category,
            std::string prod_url
           );
    
    std::string display();
    
    std::string name,
        description,
        pic_url,
        price,
        price_in_installment,
        installment_qty,
        category,
        prod_url;
};

#endif/*__PRODUCT_H__*/
