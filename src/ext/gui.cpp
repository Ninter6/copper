//
// Created by Ninter6 on 2024/7/8.
//

#include "gui.hpp"

#include <cassert>

namespace cu {

void GuiCanvas::init(const GuiInitInfo& info) {
    InitWindow(info.window_size.x, info.window_size.y, info.title);

    SetTargetFPS(60);
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    obj = std::shared_ptr<GuiCanvas>(new GuiCanvas(info.window_size));
}

bool GuiCanvas::window_should_close() {
    return WindowShouldClose();
}

std::shared_ptr<GuiCanvas> GuiCanvas::get_canvas() {
    assert(obj && "Forget to init!");
    return obj;
}

GuiCanvas::~GuiCanvas() {
    UnloadTexture(tex);
    UnloadImage(img);
    CloseWindow();
}

Image* GuiCanvas::clone() const {
    assert(false && "Not allowed to clone!");
}

GuiCanvas::GuiCanvas(const Extent& size) {
    img = GenImageColor(size.x, size.y, ::GRAY);
    tex = LoadTextureFromImage(img);
}


Extent GuiCanvas::size() const {
    return {GetScreenWidth(), GetScreenHeight()};
}

Color GuiCanvas::get(uivec2 pos) const {
    ::Color c = GetImageColor(img, (int)pos.x, (int)pos.y);
    return vec4(c.r, c.g, c.b, c.a) / 255.0f;
}

void GuiCanvas::set(uivec2 pos, const Color& color) {
    ColorU32 c = color;
    ImageDrawPixel(&img, (int)pos.x, (int)pos.y, {c.r, c.g, c.b, c.a});
}

void GuiCanvas::clear(const Color& clear_color) {
    ColorU32 c = clear_color;
    ImageClearBackground(&img, {c.r, c.g, c.b, c.a});
}

void GuiCanvas::show() const {
    UpdateTexture(tex, img.data);
    BeginDrawing();
    ClearBackground(::BLACK);
    DrawTexture(tex, 0, 0, ::WHITE);
    EndDrawing();
}

}