#include <iostream>
#include <string>

std::string extractAndStore(const std::string& route) {
    size_t lastSlashPos = route.find_last_of('/');
    
    if (lastSlashPos != std::string::npos) {
        std::string extractedString = route.substr(0, lastSlashPos);
        return extractedString;
    }
    
    return route;
}

int main() {
    std::string route1 = "Objects/test1";
    std::string route2 = "Objedrtcts/tesergergt1/egerg/teergrst3/ergregr323";
    
    std::string extractedString1 = extractAndStore(route1);
    std::string extractedString2 = extractAndStore(route2);
    
    std::cout << "추출된 문자열 1: " << extractedString1 << std::endl;
    std::cout << "추출된 문자열 2: " << extractedString2 << std::endl;
    
    return 0;
}