#ifndef __KRAWLERS_H__
#define __KRAWLERS_H__

#include <string>
#include <vector>

class KrawlerS {
public:
    KrawlerS();
    std::vector<std::ostringstream> crawl(std::vector<std::string> urls);
    std::vector<std::string> get_pages(std::string url);
};

#endif/*__KRAWLERS_H__*/
