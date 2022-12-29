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

// IValueElement
// -----------------------------------------------------------------------------
template <class T> class IValueElement : public BasicElement {
public:
  // Constructor
  IValueElement(const std::string &label = {}) : BasicElement(label) {}
  // Destructor
  virtual ~IValueElement() override {}
  // Handler
  virtual bool handle() const {
    const auto changed{_changed};
    _changed = false;
    return changed;
  }
  // Value setter
  virtual void setCurrValue(const T &value) = 0;
  // Value getter
  virtual const T &currValue() const = 0;

protected:
  mutable bool _changed{false};
};

// ValueElement
// -----------------------------------------------------------------------------
template <class T> class ValueElement : public IValueElement<T> {
public:
  // Constructor
  ValueElement(const std::string &label = {}) : IValueElement<T>(label) {}
  // Destructor
  virtual ~ValueElement() override {}
  // Value setter
  inline void setCurrValue(const T &value) override { _currValue = value; }
  // Value getter
  inline const T &currValue() const override final { return _currValue; }

protected:
  T _currValue{};

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
template <class T> class Combo : public IValueElement<T> {
public:
  // Constructor
  Combo(const std::string &label = {}) : IValueElement<T>(label) {}
  // Constructor
  Combo(const std::string &label,
        const std::vector<std::pair<T, std::string>> &valueList)
      : IValueElement<T>(label), _valueList(valueList) {}
  // Destructor
  virtual ~Combo() override {}
  // Value setter
  virtual void setCurrValue(const T &value) override {
    _currIndex = 0;
    for (int i = 0; i != _valueList.size(); ++i) {
      if (_valueList.at(i).first == value) {
        _currIndex = i;
        break;
      }
    }
  }
  // Value getter
  virtual const T &currValue() const override {
    return _valueList.at(_currIndex).first;
  }
  // Value list setter
  inline void
  setValueList(const std::vector<std::pair<T, std::string>> &valueList) {
    _valueList = valueList;
    _currIndex = std::clamp<T>(_currIndex, 0, _valueList.size() - 1);
    _currIndex = _valueList.empty() ? -1 : _currIndex;
  }
  // PLaceholder setter
  inline void setPlaceHolder(const std::string &text) { _placeholder = text; }

protected:
  std::vector<std::pair<T, std::string>> _valueList;
  int _currIndex{0};
  std::string _placeholder{};

  virtual void paintElement() override {
    _currIndex = _valueList.empty() ? -1 : _currIndex;
    if (_currIndex < 0) {
      // Value list IS empty
      if (ImGui::BeginCombo(ValueElement<T>::_label.c_str(),
                            _placeholder.c_str())) {
        ImGui::EndCombo();
      }
    } else {
      // Value list is NOT empty
      if (ImGui::BeginCombo(ValueElement<T>::_label.c_str(),
                            _valueList.at(_currIndex).second.c_str())) {
        for (size_t i = 0; i != _valueList.size(); ++i) {
          if (ImGui::Selectable(_valueList[i].second.c_str(),
                                i == _currIndex)) {
            _currIndex = i;
            IValueElement<T>::_changed = true;
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

// SpinBox2
// -----------------------------------------------------------------------------
template <class T> class SpinBoxAB : public ValueElement<std::array<T, 2>> {
public:
  // Constructor
  SpinBoxAB(const std::string &label = {})
      : ValueElement<std::array<T, 2>>(label) {}
  // Destructor
  virtual ~SpinBoxAB() override {}
  // Value A limits setter
  inline void setValueLimitsA(const std::pair<T, T> &limits) {
    _limitsA = limits;
  }
  // Value B limits setter
  inline void setValueLimitsB(const std::pair<T, T> &limits) {
    _limitsB = limits;
  }

protected:
  virtual void paintElement() override {
    paintSpinBox();
    ValueElement<std::array<T, 2>>::_currValue[0] =
        std::clamp<T>(ValueElement<std::array<T, 2>>::_currValue[0],
                      _limitsA.first, _limitsA.second);
    ValueElement<std::array<T, 2>>::_currValue[1] =
        std::clamp<T>(ValueElement<std::array<T, 2>>::_currValue[1],
                      _limitsB.first, _limitsB.second);
  }

  std::pair<T, T> _limitsA{std::numeric_limits<T>::min(),
                           std::numeric_limits<T>::max()};

  std::pair<T, T> _limitsB{std::numeric_limits<T>::min(),
                           std::numeric_limits<T>::max()};

  void paintSpinBox() {}
};

template <> inline void SpinBoxAB<int>::paintSpinBox() {
  int step{1};
  int step_fast{100};
  ImGuiInputTextFlags flags{0};
  ImGui::InputScalarN(_label.c_str(), ImGuiDataType_S32,
                      (void *)_currValue.data(), 2,
                      (void *)(step > 0 ? &step : NULL),
                      (void *)(step_fast > 0 ? &step_fast : NULL), NULL, flags);
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
