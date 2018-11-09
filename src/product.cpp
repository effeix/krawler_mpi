#include "product.hpp"

#include <iostream>
#include <sstream>


Product::Product(
        std::string name,
        std::string description,
        std::string pic_url,
        std::string price,
        std::string price_in_installment,
        std::string installment_qty,
        std::string category,
        std::string prod_url) :
    name(name),
    description(description),
    pic_url(pic_url),
    price(price),
    price_in_installment(price_in_installment),
    installment_qty(installment_qty),
    category(category),
    prod_url(prod_url) {}

std::string Product::display() {
    std::ostringstream oss;
    oss << "{" << std::endl;
    oss << "    " << "\"nome\":" << " \"" + name + "\"," << std::endl;
    oss << "    " << "\"descricao\":" << " \"" + description + "\"," << std::endl;
    oss << "    " << "\"foto\":" << " \"" + pic_url + "\"," << std::endl;
    oss << "    " << "\"preco\":" << " " + price + "," << std::endl;
    oss << "    " << "\"preco_parcelado\":" << " " + price_in_installment + "," << std::endl;
    oss << "    " << "\"preco_num_parcelas\":" << " " + installment_qty + "," << std::endl;
    oss << "    " << "\"categoria\":" << " \"" + category + "\"," << std::endl;
    oss << "    " << "\"url\":" << " \"" + prod_url + "\"," << std::endl;
    oss << "}";

    return oss.str();
}
