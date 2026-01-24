#ifndef UISTRING_HPP
#define UISTRING_HPP

#include <string>
#include "UIElement.hpp"

namespace UI {

class UIString : public UIElement {
private:
    std::string contant_;
public:
    UIString(std::array<int, 2> origin, std::array<int, 2> ending, int importance, std::string contant);
    UIString(const UIString& str);

    std::string Contant() const;
    void Contant(const std::string& contant);    
};

} //namespace

#endif // UISTRING_HPP
