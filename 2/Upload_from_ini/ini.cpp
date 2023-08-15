#include "ini.h"
using namespace std;
namespace Ini {

     Section& Document::AddSection(std::string name) {
    	 return sections[name];
     }

     const Section& Document::GetSection(const std::string& name) const {
    	 return sections.at(name);
     }

     size_t Document::SectionCount() const {
    	 return sections.size();
     }

     Document Load(std::istream& input) {
    	 Document doc;
    	 string name;
    	 size_t pos;
    	 for (string str; getline(input, str); ) {
    		 if (!str.empty()) {
        		 if (str[0] == '[') {
        			 name = str.substr(1, str.size() - 2);
        			 doc.AddSection(name);
        		 } else {
        			 pos = str.find('=');
        			 string key = str.substr(0, pos);
        			 string value = str.substr(pos + 1);
        			 doc.AddSection(name).insert({key, value});
        		 }
    		 }
    	 }
    	 return doc;
     }

}
