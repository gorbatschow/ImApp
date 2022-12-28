#pragma once
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <limits>
#include <string>
#include <vector>

namespace ImWrap {

// BasicElement
// -----------------------------------------------------------------------------
class BasicElement {
public:
  // Constructor
  BasicElement(const std::string &label = {}) : _label{label} {}
  // Destructor
  virtual ~BasicElement() {}
  // Painter
  virtual void paint() {
    ImGui::PushID(this);
    if (!std::isnan(_width))
      ImGui::SetNextItemWidth(_width);
    paintElement();
    ImGui::PopID();
  }
  // Handler
  virtual bool handle() { return false; }
  // Width setter
  inline void setWidth(const float &w) { _width = w; }
  // Label setter
  inline void setLabel(const std::string &label) { _label = label; }

protected:
  float _width{std::numeric_limits<float>::quiet_NaN()};
  std::string _label{"##"};

  virtual void paintElement() {}
};

// ValueElement
// -----------------------------------------------------------------------------
template <class T> class ValueElement : public BasicElement {
public:
  // Constructor
  ValueElement(const std::string &label = {}) : BasicElement(label) {}
  // Destructor
  virtual ~ValueElement() override {}
  // Handler
  virtual bool handle() const {
    const auto changed = _changed;
    _changed = false;
    return changed;
  }
  // Current value setter
  virtual void setCurrValue(const T &value) { _currValue = value; }
  inline const T &currValue() const { return _currValue; }

protected:
  T _currValue{};
  mutable bool _changed{false};

  virtual void paintElement() override {}
};

// Label
// -----------------------------------------------------------------------------
class Label : public BasicElement {
public:
  // Constructor
  Label(const std::string &label = {}) : BasicElement(label) {}
  // Destructor
  virtual ~Label() override {}

protected:
  virtual void paintElement() override { ImGui::Text("%s", _label.c_str()); }
};

// Button
// -----------------------------------------------------------------------------
class Button : public BasicElement {
public:
  // Constructor
  Button(const std::string &label = {}) : BasicElement(label) {}
  // Destructor
  virtual ~Button() override {}
  // Handler
  virtual bool handle() override {
    const bool trig = _triggered;
    _triggered = false;
    return trig;
  }

protected:
  bool _triggered{false};

  virtual void paintElement() override {
    _triggered = ImGui::Button(_label.c_str());
  }
};

// Combo
// -----------------------------------------------------------------------------
template <class T> class Combo : public ValueElement<T> {
public:
  // Constructor
  Combo(const std::string &label = {}) : ValueElement<T>(label) {}
  Combo(const std::string &label,
        const std::vector<std::pair<T, std::string>> &valueList)
      : ValueElement<T>(label), _valueList(valueList) {}
  // Destructor
  virtual ~Combo() override {}
  // Current value setter
  virtual void setCurrValue(const T &value) override {
    for (size_t i = 0; i != _valueList.size(); ++i) {
      if (_valueList[i].first == value) {
        _currIndex = i;
        ValueElement<T>::_currValue = value;
        break;
      }
    }
  }
  // Value list setter
  void setValueList(const std::vector<std::pair<T, std::string>> &valueList) {
    _valueList = valueList;
    _currIndex = std::clamp<T>(_currIndex, 0, _valueList.size() - 1);
    _currIndex = _valueList.empty() ? -1 : _currIndex;
  }

  inline void setPlaceHolder(const std::string &text) { _placeholder = text; }

protected:
  std::vector<std::pair<T, std::string>> _valueList;
  int _currIndex{0};
  std::string _placeholder{};

  virtual void paintElement() override {
    _currIndex = _valueList.empty() ? -1 : _currIndex;
    if (_currIndex < 0) {
      if (ImGui::BeginCombo(ValueElement<T>::_label.c_str(),
                            _placeholder.c_str())) {
        ImGui::EndCombo();
      }
    } else {
      if (ImGui::BeginCombo(ValueElement<T>::_label.c_str(),
                            _valueList.at(_currIndex).second.c_str())) {
        for (size_t i = 0; i != _valueList.size(); ++i) {
          if (ImGui::Selectable(_valueList[i].second.c_str(),
                                i == _currIndex)) {
            _currIndex = i;
            ValueElement<T>::_currValue = _valueList[i].first;
            ValueElement<T>::_changed = true;
          }
        }
        ImGui::EndCombo();
      }
    }
  }
};

// CheckBox
// -----------------------------------------------------------------------------
class CheckBox : public ValueElement<bool> {
public:
  // Constructor
  CheckBox(const std::string &label = {}) : ValueElement<bool>(label) {}
  // Destructor
  virtual ~CheckBox() override {}

protected:
  virtual void paintElement() override {
    _changed = ImGui::Checkbox(_label.c_str(), &_currValue);
  }
};

// SpinBox
// -----------------------------------------------------------------------------
template <class T> class SpinBox : public ValueElement<T> {
public:
  // Constructor
  SpinBox(const std::string &label = {}) : ValueElement<T>(label) {}
  // Destructor
  virtual ~SpinBox() override {}
  // Value limits setter
  inline void setValueLimits(const std::pair<T, T> &limits) {
    _limits = limits;
  }

protected:
  virtual void paintElement() override {
    paintSpinBox();
    ValueElement<T>::_currValue = std::clamp<T>(ValueElement<T>::_currValue,
                                                _limits.first, _limits.second);
  }

  std::pair<T, T> _limits{std::numeric_limits<T>::min(),
                          std::numeric_limits<T>::max()};

  void paintSpinBox() {}
};

template <> inline void SpinBox<int>::paintSpinBox() {
  _changed = ImGui::InputInt(_label.c_str(), &_currValue);
}

// Slider
// -----------------------------------------------------------------------------
template <class T> class Slider : public ValueElement<T> {
public:
  // Constructor
  Slider(const std::string &label = {}) : ValueElement<T>(label) {}
  // Destructor
  virtual ~Slider() override {}
  // Value limits setter
  inline void setValueLimits(const std::pair<T, T> &limits) {
    _limits = limits;
  }

protected:
  virtual void paintElement() override { paintSlider(); }

  std::pair<T, T> _limits{std::numeric_limits<T>::min() / 2,
                          std::numeric_limits<T>::max() / 2};

  void paintSlider() {}
};

template <> inline void Slider<int>::paintSlider() {
  _changed = ImGui::SliderInt(_label.c_str(), &_currValue, _limits.first,
                              _limits.second);
}

} // namespace ImWrap
