//
// Created by Ninter6 on 2025/1/6.
//

#include <cassert>

#include "async.hpp"

namespace cu {

thread_local std::vector<std::pair<ivec2, float>> AsyncPipeline::dep_buf{};
thread_local std::vector<std::pair<ivec2, Color>> AsyncPipeline::col_buf{};

AsyncPipeline::AsyncPipeline(st::ThreadPool* tp, const PipelineInitInfo& info)
    : Pipeline(info), tp(tp) {}

st::ThreadPool& AsyncPipeline::tp_or_assert() const {
    return assert(tp), *tp;
}

void AsyncPipeline::draw_buf() {
    if (!dep_buf.empty()) {
        std::unique_lock lock{dep_mutex};
        for (auto&& [p, z] : dep_buf)
            write_depth(p, z);
    }
    if (!col_buf.empty()) {
        std::unique_lock lock{col_mutex};
        for (auto&& [p, c] : col_buf)
            Pipeline::set_color(p, c);
    }
    dep_buf.clear();
    col_buf.clear();
}

void AsyncPipeline::draw_point(const Vertex& point) {
    ++remain_tasks;
    tp_or_assert().addTask([=, this] {
        Pipeline::draw_point(point);
        draw_buf();
        --remain_tasks;
    });
}

void AsyncPipeline::draw_line(const std::array<Vertex, 2>& vertices) {
    ++remain_tasks;
    tp_or_assert().addTask([=, this] {
        Pipeline::draw_line(vertices);
        draw_buf();
        --remain_tasks;
    });
}

void AsyncPipeline::draw_triangle(const std::array<Vertex, 3>& vertices) {
    ++remain_tasks;
    tp_or_assert().addTask([=, this] {
        Pipeline::draw_triangle(vertices);
        draw_buf();
        --remain_tasks;
    });
}

bool AsyncPipeline::depth_test(ivec2 pos, float z) {
    if (!depth_test_enabled())
        return true; // haven't been enabled

    float depth = 1.f / z; {
        std::shared_lock lock{dep_mutex};
        if (check_depth(pos, depth))
            return false; // failed
    }

    if (depth_write_enabled())
        dep_buf.emplace_back(pos, depth);
    return true; // passed
}

void AsyncPipeline::blend_color(ivec2 pos, Color& color) {
    std::shared_lock lock{col_mutex};
    Pipeline::blend_color(pos, color);
}

void AsyncPipeline::set_color(ivec2 pos, const Color& color) {
    col_buf.emplace_back(pos, color);
}

void AsyncPipeline::finish() const {
    while (remain_tasks != 0)
        std::this_thread::yield();
}

}
