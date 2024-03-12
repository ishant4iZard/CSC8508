#include "UIWindows.h"
#include "Vector2i.h"

UIWindows* UIWindows::ui = nullptr;

#ifdef _WIN32
const NCL::Maths::Vector4 UIBase::RED =     NCL::Maths::Vector4(255, 0, 0, 255);
const NCL::Maths::Vector4 UIBase::GREEN =   NCL::Maths::Vector4(0, 255, 0, 255);
const NCL::Maths::Vector4 UIBase::BLUE =    NCL::Maths::Vector4(0, 0, 255, 255);
const NCL::Maths::Vector4 UIBase::BLACK =   NCL::Maths::Vector4(0, 0, 0, 255);
const NCL::Maths::Vector4 UIBase::WHITE =   NCL::Maths::Vector4(255, 255, 255, 255);
const NCL::Maths::Vector4 UIBase::YELLOW =  NCL::Maths::Vector4(255, 255, 0, 255);
const NCL::Maths::Vector4 UIBase::MAGENTA = NCL::Maths::Vector4(255, 0, 255, 255);
const NCL::Maths::Vector4 UIBase::CYAN =    NCL::Maths::Vector4(0, 255, 255, 255);
#endif

UIWindows::UIWindows(){
    HWND windowHandle = NCL::Win32Code::Win32Window::windowHandle;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(windowHandle);
    ImGui_ImplOpenGL3_Init();

    // Fonts
    font = io.Fonts->AddFontFromFileTTF("../../Assets/Fonts/Roboto-Medium.ttf", 25);
    //font = io.Fonts->AddFontFromFileTTF("../../Assets/Fonts/PLANK___.ttf", 35);
    io.Fonts->Build();
}

UIWindows::~UIWindows() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

UIWindows* UIWindows::GetInstance(){
    if(ui == nullptr)
        ui = new UIWindows();

    return ui;
}

void UIWindows::Destroy() { 
    if (ui == nullptr)
        return;

    delete ui; 
    ui = nullptr;
}

void UIWindows::DrawStringText(
    const std::string& text,
    const NCL::Maths::Vector2& position,
    const NCL::Maths::Vector4& color
){
    UIElementProps* newElement = new UIElementProps();
    newElement->elementType = Text;
    newElement->text = text;
    newElement->color = color;
    newElement->position = position;

    NCL::Maths::Vector2i windowSize = NCL::Window::GetWindow()->GetScreenSize();
    newElement->position.x *= (windowSize.x/100.0f);
    newElement->position.y *= (windowSize.y/100.0f);

    uiElements.push_back(newElement);
}

void UIWindows::DrawButton(
    const std::string& text,
    const NCL::Maths::Vector2& position,
    std::function<void(void)> callback,
    const NCL::Maths::Vector4& color,
    const NCL::KeyCodes::Type& keyCode,
    const NCL::Maths::Vector2& size
) {
    UIElementProps* newElement = new UIElementProps();
    newElement->elementType = Button;
    newElement->text = text;
    newElement->color = color;
    newElement->callback = callback;
    newElement->size = size;
    newElement->position = position;

    NCL::Maths::Vector2i windowSize = NCL::Window::GetWindow()->GetScreenSize();
    newElement->position.x *= (windowSize.x / 100.0f);
    newElement->position.y *= (windowSize.y / 100.0f);

    uiElements.push_back(newElement);
}

void UIWindows::RenderUI() {
    static const ImVec4 buttonColor { 0, 0.35f, 0.65f, 1.0f };
    static const ImVec4 pressColor { 0, 0.2f, 0.8f, 1.0f };
    static const ImVec4 hoverColor { 0, 0.3f, 0.7f, 1.0f };

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::PushFont(font);

    // Next window to be created will cover the entire screen
    NCL::Maths::Vector2i windowSize = NCL::Window::GetWindow()->GetScreenSize();
    int w = windowSize.x;
    int h = windowSize.y;
    ImVec2 size(w, h);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(size);
    
    ImGui::Begin("Background", NULL,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoResize
    );
    
    for (auto uiElement : uiElements) {
        switch (uiElement->elementType) {
        case Button :
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(uiElement->color.x, uiElement->color.y, uiElement->color.z, 255));
            ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, pressColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::SetCursorScreenPos(ImVec2(uiElement->position.x, uiElement->position.y));
            if (ImGui::Button(uiElement->text.c_str(), ImVec2(uiElement->size.x, uiElement->size.y))) {
                // DO STUFF
                if (uiElement->callback != nullptr)
                    uiElement->callback();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
            break;
        case Text:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(uiElement->color.x, uiElement->color.y, uiElement->color.z, 255));
            ImGui::SetCursorScreenPos(ImVec2(uiElement->position.x, uiElement->position.y));
            ImGui::Text(uiElement->text.c_str());
            ImGui::PopStyleColor();
            break;
        default :
            break;
        }
    }

    ImGui::End();
    
    //ImGui::ShowDemoWindow(); // Only for testing

    ImGui::PopFont();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    for (auto uiElement : uiElements)
    {
        delete uiElement;
    }
    uiElements.clear();
}