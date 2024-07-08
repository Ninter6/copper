//
// Created by Ninter6 on 2024/7/8.
//

#pragma once

#include "core.hpp"

#include "raylib.h"

namespace cu {

struct GuiInitInfo {
    Extent window_size{};
    const char* title{};
};

struct GuiCanvas : public Image {
    static void init(const GuiInitInfo& info);
    static bool window_should_close();

    static std::shared_ptr<GuiCanvas> get_canvas();

    ~GuiCanvas() override;
    GuiCanvas(GuiCanvas&&) = delete;
    [[nodiscard]] Image* clone() const override;

    [[nodiscard]] Extent size() const override;
    [[nodiscard]] Color get(uivec2 pos) const override;
    void set(uivec2 pos, const Color& color) override;
    void clear(const Color& clear_color) override;

    void show() const;

    ::Image img{};
    ::Texture tex{};

private:
    explicit GuiCanvas(const Extent& size);

    static inline std::shared_ptr<GuiCanvas> obj = nullptr;
};

}
