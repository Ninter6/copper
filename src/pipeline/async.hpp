//
// Created by Ninter6 on 2025/1/6.
//

#pragma once

#include <shared_mutex>

#include "pipeline.hpp"
#include "sethread.h"

namespace cu {

class AsyncPipeline : public Pipeline {
    [[nodiscard]] st::ThreadPool& tp_or_assert() const;
    void draw_buf();

public:
    AsyncPipeline(st::ThreadPool* tp, const PipelineInitInfo& info);

    void draw_point(const Vertex& point) override;
    void draw_line(const std::array<Vertex, 2>& vertices) override;
    void draw_triangle(const std::array<Vertex, 3>& vertices) override;

    void finish() const;

protected:
    [[nodiscard]] bool depth_test(ivec2 pos, float z) override;
    void blend_color(ivec2 pos, Color& color) override;
    void set_color(ivec2 pos, const Color& color) override;

private:
    st::ThreadPool* tp{};
    std::shared_mutex dep_mutex{}; // depth
    std::shared_mutex col_mutex{}; // color
    std::atomic_size_t remain_tasks = 0;

    static thread_local std::vector<std::pair<ivec2, float>> dep_buf;
    static thread_local std::vector<std::pair<ivec2, Color>> col_buf;

};

}
