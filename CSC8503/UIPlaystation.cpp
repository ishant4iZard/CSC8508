#include "UIPlaystation.h"

UIPlaystation* UIPlaystation::ui = nullptr;

#ifdef _ORBIS 
// Static Colors
const NCL::Maths::Vector4 UIBase::RED = NCL::Debug::RED;
const NCL::Maths::Vector4 UIBase::GREEN = NCL::Debug::GREEN;
const NCL::Maths::Vector4 UIBase::BLUE = NCL::Debug::BLUE;
const NCL::Maths::Vector4 UIBase::BLACK = NCL::Debug::BLACK;
const NCL::Maths::Vector4 UIBase::WHITE = NCL::Debug::WHITE;
const NCL::Maths::Vector4 UIBase::YELLOW = NCL::Debug::YELLOW;
const NCL::Maths::Vector4 UIBase::MAGENTA = NCL::Debug::MAGENTA;
const NCL::Maths::Vector4 UIBase::CYAN = NCL::Debug::CYAN;
#endif

UIPlaystation* UIPlaystation::GetInstance()
{
	if (ui == nullptr)
		ui = new UIPlaystation;

	return ui;
}

void UIPlaystation::Destroy()
{
	if (ui == nullptr)
		return;

	delete ui;
	ui = nullptr;
}

void UIPlaystation::DrawStringText(
    const std::string& text,
    const NCL::Maths::Vector2& position,
    const NCL::Maths::Vector4& color
)
{
	UIElementProps* newElement = new UIElementProps();
	newElement->elementType = Text;
	uiElements.push_back(newElement);
	NCL::Debug::Print(text, position, color);
}

void UIPlaystation::DrawButton(
    const std::string& text,
    const NCL::Maths::Vector2& position,
    std::function<void(void)> callback,
    const NCL::Maths::Vector4& color,
    const NCL::KeyCodes::Type& keyCode,
    const NCL::Maths::Vector2& size
)
{
	UIElementProps* newElement = new UIElementProps();
	newElement->elementType = Button;
	uiElements.push_back(newElement);
	std::string displayText = text + " " + keycodeToStringMap[keyCode];
	NCL::Debug::Print(displayText, position, color);
}

void UIPlaystation::RenderUI()
{
    for (auto uiElement : uiElements) {
        switch (uiElement->elementType) {
        case Button:
			if (NCL::Window::GetKeyboard()->KeyPressed(uiElement->keyCode))
				uiElement->callback();
            break;
        case Text:
			break;
        default:
            break;
        }
    }

    for (auto uiElement : uiElements)
    {
        delete uiElement;
    }
    uiElements.clear();
}

std::unordered_map<NCL::KeyCodes::Type, std::string> keycodeToStringMap = {
    {NCL::KeyCodes::LBUTTON, "Left mouse button"},
    {NCL::KeyCodes::RBUTTON, "Right mouse button"},
    {NCL::KeyCodes::CANCEL, "Control-break processing"},
    {NCL::KeyCodes::MBUTTON, "Middle mouse button (three-button mouse)"},
    {NCL::KeyCodes::BUTTONX1, "Windows 2000/XP: X1 mouse button"},
    {NCL::KeyCodes::BUTTONX2, "Windows 2000/XP: X2 mouse button"},
    {NCL::KeyCodes::BACK, "BACKSPACE key"},
    {NCL::KeyCodes::TAB, "TAB key"},
    {NCL::KeyCodes::CLEAR, "CLEAR key"},
    {NCL::KeyCodes::RETURN, "ENTER key"},
    {NCL::KeyCodes::SHIFT, "SHIFT key"},
    {NCL::KeyCodes::CONTROL, "CTRL key"},
    {NCL::KeyCodes::MENU, "ALT key"},
    {NCL::KeyCodes::PAUSE, "PAUSE key"},
    {NCL::KeyCodes::CAPITAL, "CAPS LOCK key"},
    {NCL::KeyCodes::KANA, "IME Kana mode"},
    {NCL::KeyCodes::HANGUEL, "IME Hanguel mode (maintained for compatibility use HANGUL)"},
    {NCL::KeyCodes::HANGUL, "IME Hangul mode"},
    {NCL::KeyCodes::JUNJA, "IME Junja mode"},
    {NCL::KeyCodes::FINAL, "IME final mode"},
    {NCL::KeyCodes::HANJA, "IME Hanja mode"},
    {NCL::KeyCodes::KANJI, "IME Kanji mode"},
    {NCL::KeyCodes::ESCAPE, "ESC key"},
    {NCL::KeyCodes::CONVERT, "IME convert"},
    {NCL::KeyCodes::NONCONVERT, "IME nonconvert"},
    {NCL::KeyCodes::ACCEPT, "IME accept"},
    {NCL::KeyCodes::MODECHANGE, "IME mode change request"},
    {NCL::KeyCodes::SPACE, "SPACEBAR"},
    {NCL::KeyCodes::PRIOR, "PAGE UP key"},
    {NCL::KeyCodes::NEXT, "PAGE DOWN key"},
    {NCL::KeyCodes::END, "END key"},
    {NCL::KeyCodes::HOME, "HOME key"},
    {NCL::KeyCodes::LEFT, "LEFT ARROW key"},
    {NCL::KeyCodes::UP, "UP ARROW key"},
    {NCL::KeyCodes::RIGHT, "RIGHT ARROW key"},
    {NCL::KeyCodes::DOWN, "DOWN ARROW key"},
    {NCL::KeyCodes::SELECT, "SELECT key"},
    {NCL::KeyCodes::PRINT, "PRINT key"},
    {NCL::KeyCodes::EXECUT, "EXECUTE key"},
    {NCL::KeyCodes::SNAPSHOT, "PRINT SCREEN key"},
    {NCL::KeyCodes::INSERT, "INS key"},
    {NCL::KeyCodes::DELETEKEY, "DEL key"},
    {NCL::KeyCodes::HELP, "HELP key"},
    {NCL::KeyCodes::NUM0, "0 key"},
    {NCL::KeyCodes::NUM1, "1 key"},
    {NCL::KeyCodes::NUM2, "2 key"},
    {NCL::KeyCodes::NUM3, "3 key"},
    {NCL::KeyCodes::NUM4, "4 key"},
    {NCL::KeyCodes::NUM5, "5 key"},
    {NCL::KeyCodes::NUM6, "6 key"},
    {NCL::KeyCodes::NUM7, "7 key"},
    {NCL::KeyCodes::NUM8, "8 key"},
    {NCL::KeyCodes::NUM9, "9 key"},
    {NCL::KeyCodes::A, "A key"},
    {NCL::KeyCodes::B, "B key"},
    {NCL::KeyCodes::C, "C key"},
    {NCL::KeyCodes::D, "D key"},
    {NCL::KeyCodes::E, "E key"},
    {NCL::KeyCodes::F, "F key"},
    {NCL::KeyCodes::G, "G key"},
    {NCL::KeyCodes::H, "H key"},
    {NCL::KeyCodes::I, "I key"},
    {NCL::KeyCodes::J, "J key"},
    {NCL::KeyCodes::K, "K key"},
    {NCL::KeyCodes::L, "L key"},
    {NCL::KeyCodes::M, "M key"},
    {NCL::KeyCodes::N, "N key"},
    {NCL::KeyCodes::O, "O key"},
    {NCL::KeyCodes::P, "P key"},
    {NCL::KeyCodes::Q, "Q key"},
    {NCL::KeyCodes::R, "R key"},
    {NCL::KeyCodes::S, "S key"},
    {NCL::KeyCodes::T, "T key"},
    {NCL::KeyCodes::U, "U key"},
    {NCL::KeyCodes::V, "V key"},
    {NCL::KeyCodes::W, "W key"},
    {NCL::KeyCodes::X, "X key"},
    {NCL::KeyCodes::Y, "Y key"},
    {NCL::KeyCodes::Z, "Z key"},
    {NCL::KeyCodes::LWIN, "Left Windows key (Microsoft® Natural® keyboard)"},
    {NCL::KeyCodes::RWIN, "Right Windows key (Natural keyboard)"},
    {NCL::KeyCodes::APPS, "Applications key (Natural keyboard)"},
    {NCL::KeyCodes::SLEEP, "Computer Sleep key"},
    {NCL::KeyCodes::NUMPAD0, "Numeric keypad 0 key"},
    {NCL::KeyCodes::NUMPAD1, "Numeric keypad 1 key"},
    {NCL::KeyCodes::NUMPAD2, "Numeric keypad 2 key"},
    {NCL::KeyCodes::NUMPAD3, "Numeric keypad 3 key"},
    {NCL::KeyCodes::NUMPAD4, "Numeric keypad 4 key"},
    {NCL::KeyCodes::NUMPAD5, "Numeric keypad 5 key"},
    {NCL::KeyCodes::NUMPAD6, "Numeric keypad 6 key"},
    {NCL::KeyCodes::NUMPAD7, "Numeric keypad 7 key"},
    {NCL::KeyCodes::NUMPAD8, "Numeric keypad 8 key"},
    {NCL::KeyCodes::NUMPAD9, "Numeric keypad 9 key"},
    {NCL::KeyCodes::MULTIPLY, "Multiply key"},
    {NCL::KeyCodes::ADD, "Add key"},
    {NCL::KeyCodes::SEPARATOR, "Separator key"},
    {NCL::KeyCodes::SUBTRACT, "Subtract key"},
    {NCL::KeyCodes::DECIMAL, "Decimal key"},
    {NCL::KeyCodes::DIVIDE, "Divide key"},
    {NCL::KeyCodes::F1, "F1 key"},
    {NCL::KeyCodes::F2, "F2 key"},
    {NCL::KeyCodes::F3, "F3 key"},
    {NCL::KeyCodes::F4, "F4 key"},
    {NCL::KeyCodes::F5, "F5 key"},
    {NCL::KeyCodes::F6, "F6 key"},
    {NCL::KeyCodes::F7, "F7 key"},
    {NCL::KeyCodes::F8, "F8 key"},
    {NCL::KeyCodes::F9, "F9 key"},
    {NCL::KeyCodes::F10, "F10 key"},
    {NCL::KeyCodes::F11, "F11 key"},
    {NCL::KeyCodes::F12, "F12 key"},
    {NCL::KeyCodes::F13, "F13 key"},
    {NCL::KeyCodes::F14, "F14 key"},
    {NCL::KeyCodes::F15, "F15 key"},
    {NCL::KeyCodes::F16, "F16 key"},
    {NCL::KeyCodes::F17, "F17 key"},
    {NCL::KeyCodes::F18, "F18 key"},
    {NCL::KeyCodes::F19, "F19 key"},
    {NCL::KeyCodes::F20, "F20 key"},
    {NCL::KeyCodes::F21, "F21 key"},
    {NCL::KeyCodes::F22, "F22 key"},
    {NCL::KeyCodes::F23, "F23 key"},
    {NCL::KeyCodes::F24, "F24 key"},
    {NCL::KeyCodes::NUMLOCK, "NUM LOCK key"},
    {NCL::KeyCodes::SCROLL, "SCROLL LOCK key"},
    {NCL::KeyCodes::LSHIFT, "Left SHIFT key"},
    {NCL::KeyCodes::RSHIFT, "Right SHIFT key"},
    {NCL::KeyCodes::LCONTROL, "Left CONTROL key"},
    {NCL::KeyCodes::RCONTROL, "Right CONTROL key"},
    {NCL::KeyCodes::LMENU, "Left MENU key"},
    {NCL::KeyCodes::RMENU, "Right MENU key"},
    {NCL::KeyCodes::PLUS, "Plus Key (+)"},
    {NCL::KeyCodes::COMMA, "Comma Key (,)"},
    {NCL::KeyCodes::MINUS, "Minus Key (-)"},
    {NCL::KeyCodes::PERIOD, "Period Key (.)"},
    {NCL::KeyCodes::ATTN, "Attn key"},
    {NCL::KeyCodes::CRSEL, "CrSel key"},
    {NCL::KeyCodes::EXSEL, "ExSel key"},
    {NCL::KeyCodes::EREOF, "Erase EOF key"},
    {NCL::KeyCodes::PLAY, "Play key"},
    {NCL::KeyCodes::ZOOM, "Zoom key"},
    {NCL::KeyCodes::PA1, "PA1 key"},
    {NCL::KeyCodes::OEM_CLEAR, "Clear key"},
    {NCL::KeyCodes::MAXVALUE, "Max value"}
};