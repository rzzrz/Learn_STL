#include <iostream>
#include <memory>

class SharedResource {
public:
  SharedResource(const char *str) { this->m_string = std::make_shared<std::string>(str); }
  SharedResource(const SharedResource &shared_resource) {
    if (this == &shared_resource)
      return;
    this->m_string = shared_resource.m_string;
  }
  void read() { std::cout << *m_string << std::endl; }
  void write(const char* new_str) {
    m_string = std::make_shared<std::string>(new_str);
    std::cout<<"m_string changed"<<std::endl;
  }


private:
  std::shared_ptr<std::string> m_string;
};


int main() {

  SharedResource s1("hello");
  SharedResource s2(s1);

  s1.read();
  s2.read();

  std::cout << "开始修改" << std::endl;

  s2.write("changed str");

  std::cout << "由s2修改了字符串" << std::endl;

  s1.read();
  s2.read();
  return 0;
}