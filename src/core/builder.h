#pragma once
#include <Arduino.h>

#ifndef SETT_NO_TABLE
#include <Table.h>
#endif

#include "AnyPtr.h"
#include "build.h"
#include "colors.h"
#include "containers_class.h"
#include "logger.h"
#include "packet.h"
#include "pos.h"
#include "tmode.h"

#define _NO_ID ((size_t)(-1))

namespace sets {

enum class DivType : uint8_t {
    Default,
    Line,
    Block,
};

class Builder {
    friend class BasicContainer;

   public:
    Builder(void* settings, Build& build, sets::Packet* p = nullptr, void* db = nullptr)
        : build(build), _settings(settings), p(p), _db(db) {
        endGuest();
    }

    // инфо о билде
    Build& build;

    // авто-ID следующего виджета
    size_t nextID() {
        return _auto_id - 1;
    }

    // указатель на текущий SettingsXxx
    void* thisSettings() {
        return _settings;
    }

    // перезагрузить страницу
    void reload(bool force = false) {
        if (build.isAction()) _reload = force ? -1 : 1;
    }

    // страница будет перезагружена
    int8_t isReload() {
        return _reload;
    }

    // было действие с каким-то из виджетов выше
    bool wasSet() {
        return _was_set;
    }

    // сбросить флаг чтения wasSet
    void clearSet() {
        _was_set = false;
    }

    // ================= GUEST =================

    // разрешить неавторизованным клиентам следующий код
    bool beginGuest() {
        if (!build.granted) _enabled = true;
        return true;
    }

    // запретить неавторизованным клиентам
    void endGuest() {
        if (!build.granted) _enabled = false;
    }

    // ================= GROUP =================

    bool beginGroup(Text title = Text()) {
        return _beginContainer(Code::group, title);
    }

    void endGroup() {
        _endContainer();
    }

    // ================= ROW =================

    bool beginRow(Text title = Text(), DivType divtype = DivType::Default) {
        return _beginContainer(Code::row, title, divtype);
    }

    void endRow() {
        _endContainer();
    }

    // ================= MENU =================

    bool beginMenu(Text title) {
        return _beginContainer(Code::menu, title);
    }
    // Меню с пользовательским ID для веб-интерфейса
    bool beginMenu(Text title, Text htmlID) {
        return _beginContainer(Code::menu, title, DivType::Default, htmlID);
    }

    void endMenu() {
        _endContainer();
    }

    bool enterMenu() {
        return build.type == Build::Type::Menu && build.id == _menuID;
    }

    // ================= BUTTONS =================

    bool beginButtons() {
        return _beginContainer(Code::buttons);
    }

    void endButtons() {
        _endContainer();
    }

    // ================= LINK =================

    void Link(Text label, Text url) {
        _widget(Code::link, _NO_ID, label, &url);
    }

    void LinkTip(Text label, Text title_label, Text url) {
        _widgetTip(Code::link, _NO_ID, label, title_label, &url);
    }

    // ================= LOG =================

    void Log(size_t id, Logger& log, Text label = "") {
        if (_enabled && build.isBuild()) {
            (*p)('{');
            (*p)[Code::type] = Code::log;
            if (label.length()) (*p)[Code::label] = label;
            if (id != _NO_ID) (*p)[Code::id] = id;
            (*p)[Code::value];
            p->addLogger(log);
            (*p)('}');
            p->checkLen();
        }
    }

    void Log(Logger& log, Text label = "") {
        Log(_NO_ID, log, label);
    }

    void LogTip(size_t id, Logger& log, Text label, Text title_label) {
        if (_enabled && build.isBuild()) {
            (*p)('{');
            (*p)[Code::type] = Code::log;
            if (label.length()) (*p)[Code::label] = label;
            if (title_label.length()) (*p)[Code::title_label] = title_label;
            if (id != _NO_ID) (*p)[Code::id] = id;
            (*p)[Code::value];
            p->addLogger(log);
            (*p)('}');
            p->checkLen();
        }
    }

    void LogTip(Logger& log, Text label, Text title_label) {
        LogTip(_NO_ID, log, label, title_label);
    }

    // ================= LABEL =================

    // Старый API
    void Label(size_t id, Text label = "", Text text = Text(), uint32_t color = SETS_DEFAULT_COLOR) {
        _widget(Code::label, id, label, &text, color);
    }

    void Label(size_t id, Text label, Text text, Colors color) {
        Label(id, label, text, (uint32_t)color);
    }

    void Label(Text label = "", Text text = Text(), uint32_t color = SETS_DEFAULT_COLOR) {
        Label(_NO_ID, label, text, color);
    }

    void Label(Text label, Text text, Colors color) {
        Label(_NO_ID, label, text, (uint32_t)color);
    }

    // Новый API с tooltip
    void LabelTip(size_t id, Text label, Text title_label, Text text = Text(), uint32_t color = SETS_DEFAULT_COLOR) {
        _widgetTip(Code::label, id, label, title_label, &text, color);
    }

    void LabelTip(size_t id, Text label, Text title_label, Text text, Colors color) {
        LabelTip(id, label, title_label, text, (uint32_t)color);
    }

    void LabelTip(Text label, Text title_label, Text text = Text(), uint32_t color = SETS_DEFAULT_COLOR) {
        LabelTip(_NO_ID, label, title_label, text, color);
    }

    void LabelTip(Text label, Text title_label, Text text, Colors color) {
        LabelTip(_NO_ID, label, title_label, text, (uint32_t)color);
    }

    // ================= LABEL NUM =================

    template <typename T>
    void LabelNum(size_t id, Text label, T text, uint32_t color = SETS_DEFAULT_COLOR) {
        _widget(Code::label, id, label, &text, color);
    }

    template <typename T>
    void LabelNum(size_t id, Text label, T text, Colors color) {
        LabelNum(id, label, text, (uint32_t)color);
    }

    template <typename T>
    void LabelNum(Text label, T text, uint32_t color = SETS_DEFAULT_COLOR) {
        LabelNum(_NO_ID, label, text, color);
    }

    template <typename T>
    void LabelNum(Text label, T text, Colors color) {
        LabelNum(_NO_ID, label, text, (uint32_t)color);
    }

    template <typename T>
    void LabelNumTip(size_t id, Text label, Text title_label, T text, uint32_t color = SETS_DEFAULT_COLOR) {
        _widgetTip(Code::label, id, label, title_label, &text, color);
    }

    template <typename T>
    void LabelNumTip(size_t id, Text label, Text title_label, T text, Colors color) {
        LabelNumTip(id, label, title_label, text, (uint32_t)color);
    }

    template <typename T>
    void LabelNumTip(Text label, Text title_label, T text, uint32_t color = SETS_DEFAULT_COLOR) {
        LabelNumTip(_NO_ID, label, title_label, text, color);
    }

    template <typename T>
    void LabelNumTip(Text label, Text title_label, T text, Colors color) {
        LabelNumTip(_NO_ID, label, title_label, text, (uint32_t)color);
    }

    // ================= LABEL FLOAT =================

    void LabelFloat(size_t id, Text label, float text, uint8_t dec = 2, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_beginWidget(Code::label, id, label, &text, color)) {
            (*p)[Code::step] = dec;
            _endWidget();
        }
    }

    void LabelFloat(size_t id, Text label, float text, uint8_t dec, Colors color) {
        LabelFloat(id, label, text, dec, (uint32_t)color);
    }

    void LabelFloat(Text label, float text, uint8_t dec = 2, uint32_t color = SETS_DEFAULT_COLOR) {
        LabelFloat(_NO_ID, label, text, dec, color);
    }

    void LabelFloat(Text label, float text, uint8_t dec, Colors color) {
        LabelFloat(_NO_ID, label, text, dec, (uint32_t)color);
    }

    void LabelFloatTip(size_t id, Text label, Text title_label, float text, uint8_t dec = 2, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_beginWidgetTip(Code::label, id, label, title_label, &text, color)) {
            (*p)[Code::step] = dec;
            _endWidget();
        }
    }

    void LabelFloatTip(size_t id, Text label, Text title_label, float text, uint8_t dec, Colors color) {
        LabelFloatTip(id, label, title_label, text, dec, (uint32_t)color);
    }

    void LabelFloatTip(Text label, Text title_label, float text, uint8_t dec = 2, uint32_t color = SETS_DEFAULT_COLOR) {
        LabelFloatTip(_NO_ID, label, title_label, text, dec, color);
    }

    void LabelFloatTip(Text label, Text title_label, float text, uint8_t dec, Colors color) {
        LabelFloatTip(_NO_ID, label, title_label, text, dec, (uint32_t)color);
    }

    // ================= LED =================

    void LED(size_t id, Text label, bool value, uint32_t colorOff, uint32_t colorOn) {
        if (_beginWidget(Code::led, id, label, &value)) {
            (*p)[Code::color_off] = colorOff;
            (*p)[Code::color_on] = colorOn;
            _endWidget();
        }
    }

    void LED(size_t id, Text label, bool value, Colors colorOff, Colors colorOn) {
        LED(id, label, value, (uint32_t)colorOff, (uint32_t)colorOn);
    }

    void LED(Text label, bool value, uint32_t colorOff, uint32_t colorOn) {
        LED(_NO_ID, label, value, colorOff, colorOn);
    }

    void LED(Text label, bool value, Colors colorOff, Colors colorOn) {
        LED(_NO_ID, label, value, (uint32_t)colorOff, (uint32_t)colorOn);
    }

    void LED(size_t id, Text label, bool value) {
        _widget(Code::led, id, label, &value);
    }

    void LED(size_t id, Text label = "") {
        _widget(Code::led, id, label);
    }

    void LED(Text label, bool value) {
        LED(_NO_ID, label, value);
    }

    void LED(Text label = "") {
        LED(_NO_ID, label);
    }

    void LEDTip(size_t id, Text label, Text title_label, bool value, uint32_t colorOff, uint32_t colorOn) {
        if (_beginWidgetTip(Code::led, id, label, title_label, &value)) {
            (*p)[Code::color_off] = colorOff;
            (*p)[Code::color_on] = colorOn;
            _endWidget();
        }
    }

    void LEDTip(size_t id, Text label, Text title_label, bool value, Colors colorOff, Colors colorOn) {
        LEDTip(id, label, title_label, value, (uint32_t)colorOff, (uint32_t)colorOn);
    }

    void LEDTip(Text label, Text title_label, bool value, uint32_t colorOff, uint32_t colorOn) {
        LEDTip(_NO_ID, label, title_label, value, colorOff, colorOn);
    }

    void LEDTip(Text label, Text title_label, bool value, Colors colorOff, Colors colorOn) {
        LEDTip(_NO_ID, label, title_label, value, (uint32_t)colorOff, (uint32_t)colorOn);
    }

    void LEDTip(size_t id, Text label, Text title_label, bool value) {
        _widgetTip(Code::led, id, label, title_label, &value);
    }

    void LEDTip(size_t id, Text label, Text title_label) {
        _widgetTip(Code::led, id, label, title_label);
    }

    void LEDTip(Text label, Text title_label, bool value) {
        LEDTip(_NO_ID, label, title_label, value);
    }

    void LEDTip(Text label, Text title_label) {
        LEDTip(_NO_ID, label, title_label);
    }

    // ================= IMAGE =================

    void Image(size_t id, Text label, Text path) {
        _widget(Code::image, id, label, &path);
    }

    void Image(Text label, Text path) {
        Image(_NO_ID, label, path);
    }

    void ImageTip(size_t id, Text label, Text title_label, Text path) {
        _widgetTip(Code::image, id, label, title_label, &path);
    }

    void ImageTip(Text label, Text title_label, Text path) {
        ImageTip(_NO_ID, label, title_label, path);
    }

    // ================= STREAM =================

    void Stream() {
        if (_enabled && build.isBuild()) {
            (*p)('{');
            (*p)[Code::type] = Code::stream;
            _endWidget();
        }
    }

    // ================= TEXT =================

    void Paragraph(size_t id, Text label = "", Text text = Text()) {
        _widget(Code::paragraph, id, label, &text);
    }

    void Paragraph(Text label = "", Text text = Text()) {
        Paragraph(_NO_ID, label, text);
    }

    void ParagraphTip(size_t id, Text label, Text title_label, Text text = Text()) {
        _widgetTip(Code::paragraph, id, label, title_label, &text);
    }

    void ParagraphTip(Text label, Text title_label, Text text = Text()) {
        ParagraphTip(_NO_ID, label, title_label, text);
    }

    // ================= HTML =================

    void HTML(size_t id, Text label = "", Text html = Text()) {
        _widget(Code::html, id, label, &html);
    }

    void HTML(Text label = "", Text html = Text()) {
        HTML(_NO_ID, label, html);
    }

    // ================= TABLE =================

    void Table(size_t id, Text table, Text labels = Text()) {
        _widget(Code::table, id, labels, &table);
    }

    void Table(Text table, Text labels = Text()) {
        Table(_NO_ID, table, labels);
    }

    void TableTip(size_t id, Text table, Text labels, Text title_labels) {
        _widgetTip(Code::table, id, labels, title_labels, &table);
    }

    void TableTip(Text table, Text labels, Text title_labels) {
        TableTip(_NO_ID, table, labels, title_labels);
    }

#ifndef SETT_NO_TABLE

    void Table(size_t id, ::Table& table, Text labels = Text()) {
        if (_beginWidget(Code::table, id, labels)) {
            (*p)[Code::value];
            p->beginBin(table.writeSize());
            table.writeTo(*p);
            _endWidget();
        }
    }

    void Table(::Table& table, Text labels = Text()) {
        Table(_NO_ID, table, labels);
    }

    void TableTip(size_t id, ::Table& table, Text labels, Text title_labels) {
        if (_beginWidgetTip(Code::table, id, labels, title_labels)) {
            (*p)[Code::value];
            p->beginBin(table.writeSize());
            table.writeTo(*p);
            _endWidget();
        }
    }

    void TableTip(::Table& table, Text labels, Text title_labels) {
        TableTip(_NO_ID, table, labels, title_labels);
    }

#endif

    // ================= GAUGE =================

    void LinearGauge(size_t id, Text label = "", float min = 0, float max = 100, Text unit = Text(), float value = NAN, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_beginWidget(Code::gauge, id, label, isnan(value) ? nullptr : &value, color)) {
            (*p)[Code::min] = min;
            (*p)[Code::max] = max;
            (*p)[Code::unit] = unit;
            _endWidget();
        }
    }

    void LinearGauge(size_t id, Text label, float min, float max, Text unit, float value, Colors color) {
        LinearGauge(id, label, min, max, unit, value, (uint32_t)color);
    }

    void LinearGauge(Text label = "", float min = 0, float max = 100, Text unit = Text(), float value = NAN, uint32_t color = SETS_DEFAULT_COLOR) {
        LinearGauge(_NO_ID, label, min, max, unit, value, color);
    }

    void LinearGauge(Text label, float min, float max, Text unit, float value, Colors color) {
        LinearGauge(_NO_ID, label, min, max, unit, value, (uint32_t)color);
    }

    void LinearGaugeTip(size_t id, Text label, Text title_label, float min = 0, float max = 100, Text unit = Text(), float value = NAN, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_beginWidgetTip(Code::gauge, id, label, title_label, isnan(value) ? nullptr : &value, color)) {
            (*p)[Code::min] = min;
            (*p)[Code::max] = max;
            (*p)[Code::unit] = unit;
            _endWidget();
        }
    }

    void LinearGaugeTip(size_t id, Text label, Text title_label, float min, float max, Text unit, float value, Colors color) {
        LinearGaugeTip(id, label, title_label, min, max, unit, value, (uint32_t)color);
    }

    void LinearGaugeTip(Text label, Text title_label, float min = 0, float max = 100, Text unit = Text(), float value = NAN, uint32_t color = SETS_DEFAULT_COLOR) {
        LinearGaugeTip(_NO_ID, label, title_label, min, max, unit, value, color);
    }

    void LinearGaugeTip(Text label, Text title_label, float min, float max, Text unit, float value, Colors color) {
        LinearGaugeTip(_NO_ID, label, title_label, min, max, unit, value, (uint32_t)color);
    }

    // ================= PLOT =================

    void PlotRunning(size_t id, Text labels = Text(), uint16_t period = 200) {
        if (_beginWidget(Code::plot_run, id, labels)) {
            (*p)[Code::period] = period;
            _endWidget();
        }
    }

    void PlotRunning(Text labels = Text(), uint16_t period = 200) {
        PlotRunning(_NO_ID, labels, period);
    }

    void PlotRunningTip(size_t id, Text labels, Text title_labels, uint16_t period = 200) {
        if (_beginWidgetTip(Code::plot_run, id, labels, title_labels)) {
            (*p)[Code::period] = period;
            _endWidget();
        }
    }

    void PlotRunningTip(Text labels, Text title_labels, uint16_t period = 200) {
        PlotRunningTip(_NO_ID, labels, title_labels, period);
    }

    void PlotStack(size_t id, Text labels = Text()) {
        _widget(Code::plot_stack, id, labels);
    }

    void PlotStack(Text labels = Text()) {
        PlotStack(_NO_ID, labels);
    }

    void PlotStackTip(size_t id, Text labels, Text title_labels) {
        _widgetTip(Code::plot_stack, id, labels, title_labels);
    }

    void PlotStackTip(Text labels, Text title_labels) {
        PlotStackTip(_NO_ID, labels, title_labels);
    }

    void Plot(size_t id, Text path = Text(), Text labels = Text()) {
        _widget(Code::plot, id, labels, &path);
    }

    void Plot(Text path = Text(), Text labels = Text()) {
        Plot(_NO_ID, path, labels);
    }

    void PlotTip(size_t id, Text path, Text labels, Text title_labels) {
        _widgetTip(Code::plot, id, labels, title_labels, &path);
    }

    void PlotTip(Text path, Text labels, Text title_labels) {
        PlotTip(_NO_ID, path, labels, title_labels);
    }

#ifndef SETT_NO_TABLE

    void Plot(size_t id, ::Table& table, Text labels = Text(), bool clearTable = true) {
        if (_beginWidget(Code::plot, id, labels)) {
            (*p)[Code::value];
            p->beginBin(table.writeSize());
            table.writeTo(*p);
            if (clearTable) table.removeAll();
            _endWidget();
        }
    }

    void Plot(::Table& table, Text labels = Text(), bool clearTable = true) {
        Plot(_NO_ID, table, labels, clearTable);
    }

    void PlotTip(size_t id, ::Table& table, Text labels, Text title_labels, bool clearTable = true) {
        if (_beginWidgetTip(Code::plot, id, labels, title_labels)) {
            (*p)[Code::value];
            p->beginBin(table.writeSize());
            table.writeTo(*p);
            if (clearTable) table.removeAll();
            _endWidget();
        }
    }

    void PlotTip(::Table& table, Text labels, Text title_labels, bool clearTable = true) {
        PlotTip(_NO_ID, table, labels, title_labels, clearTable);
    }

#endif

    void PlotTimeline(size_t id, Text path, TMode mode, Text labels) {
        if (_beginWidget(Code::plot_time, id, labels, &path)) {
            (*p)[Code::tmode] = (uint8_t)mode;
            _endWidget();
        }
    }

    void PlotTimeline(Text path, TMode mode, Text labels) {
        PlotTimeline(_NO_ID, path, mode, labels);
    }

    void PlotTimelineTip(size_t id, Text path, TMode mode, Text labels, Text title_labels) {
        if (_beginWidgetTip(Code::plot_time, id, labels, title_labels, &path)) {
            (*p)[Code::tmode] = (uint8_t)mode;
            _endWidget();
        }
    }

    void PlotTimelineTip(Text path, TMode mode, Text labels, Text title_labels) {
        PlotTimelineTip(_NO_ID, path, mode, labels, title_labels);
    }

#ifndef SETT_NO_TABLE

    void PlotTimeline(size_t id, ::Table& table, TMode mode, Text labels, bool clearTable = true) {
        if (_beginWidget(Code::plot, id, labels)) {
            (*p)[Code::tmode] = (uint8_t)mode;
            (*p)[Code::value];
            p->beginBin(table.writeSize());
            table.writeTo(*p);
            if (clearTable) table.removeAll();
            _endWidget();
        }
    }

    void PlotTimeline(::Table& table, TMode mode, Text labels, bool clearTable = true) {
        PlotTimeline(_NO_ID, table, mode, labels, clearTable);
    }

    void PlotTimelineTip(size_t id, ::Table& table, TMode mode, Text labels, Text title_labels, bool clearTable = true) {
        if (_beginWidgetTip(Code::plot, id, labels, title_labels)) {
            (*p)[Code::tmode] = (uint8_t)mode;
            (*p)[Code::value];
            p->beginBin(table.writeSize());
            table.writeTo(*p);
            if (clearTable) table.removeAll();
            _endWidget();
        }
    }

    void PlotTimelineTip(::Table& table, TMode mode, Text labels, Text title_labels, bool clearTable = true) {
        PlotTimelineTip(_NO_ID, table, mode, labels, title_labels, clearTable);
    }

#endif

    // ================= JOYSTICK =================

    bool Joystick(Pos& pos, bool center = false) {
        size_t id = _next();
        if (_beginWidget(Code::joystick, id, Text())) {
            (*p)[Code::center] = center;
            _endWidget();
        }
        return _isSet(id, &pos);
    }

    // ================= INPUT =================

    bool Input(size_t id, Text label = "", AnyPtr value = nullptr, Text regex = Text(), Text format = Text()) {
        if (_beginWidget(Code::input, id, label, value)) {
            if (regex) (*p)[Code::regex] = regex;
            if (format) (*p)[Code::format] = format;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool Input(Text label = "", AnyPtr value = nullptr, Text regex = Text(), Text format = Text()) {
        return Input(_next(), label, value, regex, format);
    }

    bool InputTip(size_t id, Text label, Text title_label, AnyPtr value = nullptr, Text regex = Text(), Text format = Text()) {
        if (_beginWidgetTip(Code::input, id, label, title_label, value)) {
            if (regex) (*p)[Code::regex] = regex;
            if (format) (*p)[Code::format] = format;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool InputTip(Text label, Text title_label, AnyPtr value = nullptr, Text regex = Text(), Text format = Text()) {
        return InputTip(_next(), label, title_label, value, regex, format);
    }

    // ================= NUMBER =================

    bool Number(size_t id, Text label = "", AnyPtr value = nullptr, float min = NAN, float max = NAN) {
        if (_beginWidget(Code::number, id, label, value)) {
            if (!isnan(min)) (*p)[Code::min] = min;
            if (!isnan(max)) (*p)[Code::max] = max;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool Number(Text label = "", AnyPtr value = nullptr, float min = NAN, float max = NAN) {
        return Number(_next(), label, value, min, max);
    }

    bool NumberTip(size_t id, Text label, Text title_label, AnyPtr value = nullptr, float min = NAN, float max = NAN) {
        if (_beginWidgetTip(Code::number, id, label, title_label, value)) {
            if (!isnan(min)) (*p)[Code::min] = min;
            if (!isnan(max)) (*p)[Code::max] = max;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool NumberTip(Text label, Text title_label, AnyPtr value = nullptr, float min = NAN, float max = NAN) {
        return NumberTip(_next(), label, title_label, value, min, max);
    }

    // ================= PASS =================

    bool Pass(size_t id, Text label = "", AnyPtr value = nullptr, Text regex = Text(), Text format = Text()) {
        if (_beginWidget(Code::pass, id, label, value)) {
            if (regex) (*p)[Code::regex] = regex;
            if (format) (*p)[Code::format] = format;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool Pass(Text label = "", AnyPtr value = nullptr, Text regex = Text(), Text format = Text()) {
        return Pass(_next(), label, value, regex, format);
    }

    bool PassTip(size_t id, Text label, Text title_label, AnyPtr value = nullptr, Text regex = Text(), Text format = Text()) {
        if (_beginWidgetTip(Code::pass, id, label, title_label, value)) {
            if (regex) (*p)[Code::regex] = regex;
            if (format) (*p)[Code::format] = format;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool PassTip(Text label, Text title_label, AnyPtr value = nullptr, Text regex = Text(), Text format = Text()) {
        return PassTip(_next(), label, title_label, value, regex, format);
    }

    // ================= COLOR =================

    bool Color(size_t id, Text label = "", uint32_t* value = nullptr) {
        _widget(Code::color, id, label, value);
        return _isSet(id, value);
    }

    bool Color(Text label = "", uint32_t* value = nullptr) {
        return Color(_next(), label, value);
    }

    bool ColorTip(size_t id, Text label, Text title_label, uint32_t* value = nullptr) {
        _widgetTip(Code::color, id, label, title_label, value);
        return _isSet(id, value);
    }

    bool ColorTip(Text label, Text title_label, uint32_t* value = nullptr) {
        return ColorTip(_next(), label, title_label, value);
    }

    // ================= SWITCH =================

    bool Switch(size_t id, Text label = "", bool* value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        _widget(Code::toggle, id, label, value, color);
        return _isSet(id, value);
    }

    bool Switch(size_t id, Text label, bool* value, Colors color) {
        return Switch(id, label, value, (uint32_t)color);
    }

    bool Switch(Text label = "", bool* value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        return Switch(_next(), label, value, color);
    }

    bool Switch(Text label, bool* value, Colors color) {
        return Switch(_next(), label, value, (uint32_t)color);
    }

    bool SwitchTip(size_t id, Text label, Text title_label, bool* value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        _widgetTip(Code::toggle, id, label, title_label, value, color);
        return _isSet(id, value);
    }

    bool SwitchTip(size_t id, Text label, Text title_label, bool* value, Colors color) {
        return SwitchTip(id, label, title_label, value, (uint32_t)color);
    }

    bool SwitchTip(Text label, Text title_label, bool* value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        return SwitchTip(_next(), label, title_label, value, color);
    }

    bool SwitchTip(Text label, Text title_label, bool* value, Colors color) {
        return SwitchTip(_next(), label, title_label, value, (uint32_t)color);
    }

    // ================= DATE =================

    bool Date(size_t id, Text label = "", uint32_t* value = nullptr, float zone_hours = NAN) {
        if (_beginWidget(Code::date, id, label, value)) {
            if (!isnan(zone_hours)) (*p)[Code::zone] = round(zone_hours * 60);
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool Date(Text label = "", uint32_t* value = nullptr, float zone_hours = NAN) {
        return Date(_next(), label, value, zone_hours);
    }

    bool DateTip(size_t id, Text label, Text title_label, uint32_t* value = nullptr, float zone_hours = NAN) {
        if (_beginWidgetTip(Code::date, id, label, title_label, value)) {
            if (!isnan(zone_hours)) (*p)[Code::zone] = round(zone_hours * 60);
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool DateTip(Text label, Text title_label, uint32_t* value = nullptr, float zone_hours = NAN) {
        return DateTip(_next(), label, title_label, value, zone_hours);
    }

    // ================= TIME =================

    bool Time(size_t id, Text label = "", uint32_t* value = nullptr) {
        _widget(Code::time, id, label, value);
        return _isSet(id, value);
    }

    bool Time(Text label = "", uint32_t* value = nullptr) {
        return Time(_next(), label, value);
    }

    bool TimeTip(size_t id, Text label, Text title_label, uint32_t* value = nullptr) {
        _widgetTip(Code::time, id, label, title_label, value);
        return _isSet(id, value);
    }

    bool TimeTip(Text label, Text title_label, uint32_t* value = nullptr) {
        return TimeTip(_next(), label, title_label, value);
    }

    // ================= DATETIME =================

    bool DateTime(size_t id, Text label = "", uint32_t* value = nullptr, float zone_hours = NAN) {
        if (_beginWidget(Code::datetime, id, label, value)) {
            if (!isnan(zone_hours)) (*p)[Code::zone] = round(zone_hours * 60);
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool DateTime(Text label = "", uint32_t* value = nullptr, float zone_hours = NAN) {
        return DateTime(_next(), label, value, zone_hours);
    }

    bool DateTimeTip(size_t id, Text label, Text title_label, uint32_t* value = nullptr, float zone_hours = NAN) {
        if (_beginWidgetTip(Code::datetime, id, label, title_label, value)) {
            if (!isnan(zone_hours)) (*p)[Code::zone] = round(zone_hours * 60);
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool DateTimeTip(Text label, Text title_label, uint32_t* value = nullptr, float zone_hours = NAN) {
        return DateTimeTip(_next(), label, title_label, value, zone_hours);
    }

    // ================= SPINNER =================

    bool Spinner(size_t id, Text label = "", float min = 0, float max = 100, float step = 1, AnyPtr value = nullptr) {
        if (_beginWidget(Code::spinner, id, label, value)) {
            if (!isnan(min)) (*p)[Code::min] = min;
            if (!isnan(max)) (*p)[Code::max] = max;
            (*p)[Code::step] = step;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool Spinner(Text label = "", float min = 0, float max = 100, float step = 1, AnyPtr value = nullptr) {
        return Spinner(_next(), label, min, max, step, value);
    }

    bool SpinnerTip(size_t id, Text label, Text title_label, float min = 0, float max = 100, float step = 1, AnyPtr value = nullptr) {
        if (_beginWidgetTip(Code::spinner, id, label, title_label, value)) {
            if (!isnan(min)) (*p)[Code::min] = min;
            if (!isnan(max)) (*p)[Code::max] = max;
            (*p)[Code::step] = step;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool SpinnerTip(Text label, Text title_label, float min = 0, float max = 100, float step = 1, AnyPtr value = nullptr) {
        return SpinnerTip(_next(), label, title_label, min, max, step, value);
    }

    // ================= SLIDER =================

    bool Slider(size_t id, Text label = "", float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_beginWidget(Code::slider, id, label, value, color)) {
            (*p)[Code::min] = min;
            (*p)[Code::max] = max;
            (*p)[Code::step] = step;
            if (unit.length()) (*p)[Code::unit] = unit;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool Slider(size_t id, Text label, float min, float max, float step, Text unit, AnyPtr value, Colors color) {
        return Slider(id, label, min, max, step, unit, value, (uint32_t)color);
    }

    bool Slider(Text label = "", float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        return Slider(_next(), label, min, max, step, unit, value, color);
    }

    bool Slider(Text label, float min, float max, float step, Text unit, AnyPtr value, Colors color) {
        return Slider(_next(), label, min, max, step, unit, value, (uint32_t)color);
    }

    bool SliderTip(size_t id, Text label, Text title_label, float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_beginWidgetTip(Code::slider, id, label, title_label, value, color)) {
            (*p)[Code::min] = min;
            (*p)[Code::max] = max;
            (*p)[Code::step] = step;
            if (unit.length()) (*p)[Code::unit] = unit;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool SliderTip(size_t id, Text label, Text title_label, float min, float max, float step, Text unit, AnyPtr value, Colors color) {
        return SliderTip(id, label, title_label, min, max, step, unit, value, (uint32_t)color);
    }

    bool SliderTip(Text label, Text title_label, float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        return SliderTip(_next(), label, title_label, min, max, step, unit, value, color);
    }

    bool SliderTip(Text label, Text title_label, float min, float max, float step, Text unit, AnyPtr value, Colors color) {
        return SliderTip(_next(), label, title_label, min, max, step, unit, value, (uint32_t)color);
    }

    // ================= SLIDER2 =================

    bool Slider2(size_t id_min, size_t id_max, Text label = "", float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value_min = nullptr, AnyPtr value_max = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_enabled && build.isBuild()) {
            (*p)('{');
            (*p)[Code::type] = Code::slider2;
            if (label.length()) (*p)[Code::label] = label;
            if (color != SETS_DEFAULT_COLOR) (*p)[Code::color] = color;
            (*p)[Code::min] = min;
            (*p)[Code::max] = max;
            (*p)[Code::step] = step;
            if (unit.length()) (*p)[Code::unit] = unit;
            (*p)[Code::id] = id_min;
            (*p)[Code::id2] = id_max;

            if ((*p)[Code::value]('[')) {
                if (value_min) value_min.write(p);
                else if (_db) p->addFromDB(_db, id_min);

                if (value_max) value_max.write(p);
                else if (_db) p->addFromDB(_db, id_max);
                (*p)(']');
            }

            (*p)('}');
            p->checkLen();
        }

        bool set = _isSet(id_min, value_min);
        set |= _isSet(id_max, value_max);
        return set;
    }

    bool Slider2(size_t id_min, size_t id_max, Text label, float min, float max, float step, Text unit, AnyPtr value_min, AnyPtr value_max, Colors color) {
        return Slider2(id_min, id_max, label, min, max, step, unit, value_min, value_max, (uint32_t)color);
    }

    bool Slider2(Text label = "", float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value_min = nullptr, AnyPtr value_max = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        return Slider2(_next(), _next(), label, min, max, step, unit, value_min, value_max, color);
    }

    bool Slider2(Text label, float min, float max, float step, Text unit, AnyPtr value_min, AnyPtr value_max, Colors color) {
        return Slider2(_next(), _next(), label, min, max, step, unit, value_min, value_max, (uint32_t)color);
    }

    bool Slider2Tip(size_t id_min, size_t id_max, Text label, Text title_label, float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value_min = nullptr, AnyPtr value_max = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_enabled && build.isBuild()) {
            (*p)('{');
            (*p)[Code::type] = Code::slider2;
            if (label.length()) (*p)[Code::label] = label;
            if (title_label.length()) (*p)[Code::title_label] = title_label;
            if (color != SETS_DEFAULT_COLOR) (*p)[Code::color] = color;
            (*p)[Code::min] = min;
            (*p)[Code::max] = max;
            (*p)[Code::step] = step;
            if (unit.length()) (*p)[Code::unit] = unit;
            (*p)[Code::id] = id_min;
            (*p)[Code::id2] = id_max;

            if ((*p)[Code::value]('[')) {
                if (value_min) value_min.write(p);
                else if (_db) p->addFromDB(_db, id_min);

                if (value_max) value_max.write(p);
                else if (_db) p->addFromDB(_db, id_max);
                (*p)(']');
            }

            (*p)('}');
            p->checkLen();
        }

        bool set = _isSet(id_min, value_min);
        set |= _isSet(id_max, value_max);
        return set;
    }

    bool Slider2Tip(size_t id_min, size_t id_max, Text label, Text title_label, float min, float max, float step, Text unit, AnyPtr value_min, AnyPtr value_max, Colors color) {
        return Slider2Tip(id_min, id_max, label, title_label, min, max, step, unit, value_min, value_max, (uint32_t)color);
    }

    bool Slider2Tip(Text label, Text title_label, float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value_min = nullptr, AnyPtr value_max = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        return Slider2Tip(_next(), _next(), label, title_label, min, max, step, unit, value_min, value_max, color);
    }

    bool Slider2Tip(Text label, Text title_label, float min, float max, float step, Text unit, AnyPtr value_min, AnyPtr value_max, Colors color) {
        return Slider2Tip(_next(), _next(), label, title_label, min, max, step, unit, value_min, value_max, (uint32_t)color);
    }

    // ================= SELECT =================

    bool Select(size_t id, Text label, Text options, AnyPtr value = nullptr) {
        return _Select(id, label, options, value, false);
    }

    bool Select(Text label, Text options, AnyPtr value = nullptr) {
        return _Select(_next(), label, options, value, false);
    }

    bool SelectTip(size_t id, Text label, Text title_label, Text options, AnyPtr value = nullptr) {
        return _SelectTip(id, label, title_label, options, value, false);
    }

    bool SelectTip(Text label, Text title_label, Text options, AnyPtr value = nullptr) {
        return _SelectTip(_next(), label, title_label, options, value, false);
    }

    bool SelectText(Text label, Text options) {
        return _Select(_next(), label, options, nullptr, true);
    }

    bool SelectTextTip(Text label, Text title_label, Text options) {
        return _SelectTip(_next(), label, title_label, options, nullptr, true);
    }

    bool _Select(size_t id, Text label, Text options, AnyPtr value, bool asText) {
        if (_beginWidget(Code::select, id, label, value)) {
            (*p)[Code::text] = options;
            (*p)[Code::as_txt] = asText;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool _SelectTip(size_t id, Text label, Text title_label, Text options, AnyPtr value, bool asText) {
        if (_beginWidgetTip(Code::select, id, label, title_label, value)) {
            (*p)[Code::text] = options;
            (*p)[Code::as_txt] = asText;
            _endWidget();
        }
        return _isSet(id, value);
    }

    // ================= TABS =================

    bool Tabs(size_t id, Text tabs, AnyPtr value = nullptr) {
        if (_beginWidget(Code::tabs, id, Text(), value)) {
            (*p)[Code::text] = tabs;
            _endWidget();
        }
        return _isSet(id, value);
    }

    bool Tabs(Text tabs, AnyPtr value = nullptr) {
        return Tabs(_next(), tabs, value);
    }

    // ================= BUTTON =================

    bool Button(size_t id, Text label = "", uint32_t color = SETS_DEFAULT_COLOR) {
        _widget(Code::button, id, label, nullptr, color);
        return _isSet(id, nullptr);
    }

    bool Button(Text label = "", uint32_t color = SETS_DEFAULT_COLOR) {
        return Button(_next(), label, color);
    }

    bool Button(size_t id, Text label, Colors color) {
        return Button(id, label, (uint32_t)color);
    }

    bool Button(Text label, Colors color) {
        return Button(_next(), label, (uint32_t)color);
    }

    // ================= BUTTON HOLD =================

    bool ButtonHold(size_t id, Text label = "", uint32_t color = SETS_DEFAULT_COLOR) {
        if (_beginWidget(Code::button, id, label, nullptr, color)) {
            (*p)[Code::button_hold] = true;
            _endWidget();
        }
        return _isSet(id, nullptr);
    }

    bool ButtonHold(Text label = "", uint32_t color = SETS_DEFAULT_COLOR) {
        return ButtonHold(_next(), label, color);
    }

    bool ButtonHold(size_t id, Text label, Colors color) {
        return ButtonHold(id, label, (uint32_t)color);
    }

    bool ButtonHold(Text label, Colors color) {
        return ButtonHold(_next(), label, (uint32_t)color);
    }

    // ================= CONFIRM =================

    bool Confirm(size_t id, Text label = "", bool* ptr = nullptr) {
        _widget(Code::confirm, id, label);
        return _isSet(id, ptr);
    }

    bool ConfirmTip(size_t id, Text label, Text title_label, uint32_t color = SETS_DEFAULT_COLOR, bool* ptr = nullptr) {
        _widgetTip(Code::confirm, id, label, title_label, nullptr, color);
        return _isSet(id, ptr);
    }

    bool ConfirmTip(size_t id, Text label, Text title_label, Colors color, bool* ptr = nullptr) {
        return ConfirmTip(id, label, title_label, (uint32_t)color, ptr);
    }

    // ================= CUSTOM =================

    bool Custom(Text type, size_t id, const BSON& params = BSON(), AnyPtr value = nullptr) {
        if (_enabled && build.isBuild()) {
            (*p)('{');
            (*p)[Code::id] = id;
            (*p)[Code::type] = type;
            p->add(params);

            if (value) {
                (*p)[Code::value];
                value.write(p);
                if (value.type() == AnyPtr::Type::Char && value.len()) {
                    (*p)[Code::maxlen] = value.len() - 1;
                }
            } else if (_db && p->inDB(_db, id)) {
                (*p)[Code::value];
                p->addFromDB(_db, id);
            }

            (*p)('}');
            p->checkLen();
            return _isSet(id, value);
        }
        return false;
    }

    // ================= TABLE OLD =================

    void TableCSV(size_t id, Text csv, Text labels = Text()) {
        _widget(Code::table, id, labels, &csv);
    }

    void TableCSV(Text csv, Text labels = Text()) {
        TableCSV(_NO_ID, csv, labels);
    }

    void TableCSVTip(size_t id, Text csv, Text labels, Text title_labels) {
        _widgetTip(Code::table, id, labels, title_labels, &csv);
    }

    void TableCSVTip(Text csv, Text labels, Text title_labels) {
        TableCSVTip(_NO_ID, csv, labels, title_labels);
    }

   private:

    void* _settings;
    sets::Packet* p = nullptr;
    void* _db = nullptr;

    size_t _auto_id = UINT32_MAX;

    int8_t _reload = 0;
    uint8_t _menuID = 0;

    bool _enabled = true;
    bool _was_set = false;
    bool _set_f = false;

    size_t _next() {
        return --_auto_id;
    }

    // ================= WIDGET HELPERS =================

    bool _widget(Code type, size_t id, Text& label, AnyPtr value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_beginWidget(type, id, label, value, color)) {
            _endWidget();
            return true;
        }
        return false;
    }

    bool _widgetTip(Code type, size_t id, const Text& label, const Text& title_label, AnyPtr value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_beginWidgetTip(type, id, label, title_label, value, color)) {
            _endWidget();
            return true;
        }
        return false;
    }

    bool _beginWidget(Code type, size_t id, const Text& label, AnyPtr value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        return _beginWidgetTip(type, id, label, Text(), value, color);
    }

    bool _beginWidgetTip(Code type, size_t id, const Text& label, const Text& title_label, AnyPtr value = nullptr, uint32_t color = SETS_DEFAULT_COLOR) {
        if (_enabled && build.isBuild()) {
            (*p)('{');

            (*p)[Code::type] = type;

            if (label.length()) (*p)[Code::label] = label;
            if (title_label.length()) (*p)[Code::title_label] = title_label;

            if (color != SETS_DEFAULT_COLOR) (*p)[Code::color] = color;

            if (id != _NO_ID) (*p)[Code::id] = id;

            if (value) {
                (*p)[Code::value];
                value.write(p);

                if (value.type() == AnyPtr::Type::Char && value.len()) {
                    (*p)[Code::maxlen] = value.len() - 1;
                }
            } else if (_db && id != _NO_ID && p->inDB(_db, id)) {
                (*p)[Code::value];
                p->addFromDB(_db, id);
            }

            return true;
        }

        return false;
    }

    void _endWidget() {
        (*p)('}');
        p->checkLen();
    }

    // ================= CONTAINERS =================

    bool _beginContainer(Code type, Text title = Text(), DivType divtype = DivType::Default, Text htmlID = Text()) {
        if (type == Code::menu) ++_menuID;

        if (build.isBuild()) {
            (*p)('{');
            (*p)[Code::type] = type;

            if (type == Code::menu) (*p)[Code::id] = _menuID;
            if (title.length()) (*p)[Code::title] = title;
            if (htmlID.length()) (*p)[Code::html_id] = htmlID;

            switch (divtype) {
                case DivType::Default:
                    break;

                case DivType::Line:
                    (*p)[Code::divtype] = Code::line;
                    break;

                case DivType::Block:
                    (*p)[Code::divtype] = Code::block;
                    break;
            }

            (*p)[Code::content]('[');
        }

        return true;
    }

    void _endContainer() {
        if (build.isBuild()) {
            (*p)(']');
            (*p)('}');
        }
    }

    // ================= ACTION =================

    bool _isSet(size_t id, AnyPtr value) {
        bool set = (!_set_f && _enabled && build.isAction() && id == build.id);

        if (set) _set_f = true;

        if (value && set) value.read(build.value);

        _was_set |= set;

        return set;
    }
};

}  // namespace sets