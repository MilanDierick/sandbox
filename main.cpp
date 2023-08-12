#include <iostream>

#include <flecs.h>
#include <glm/glm.hpp>

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

using b8 = bool;

using fvec3 = glm::fvec3;
using fvec4 = glm::fvec4;

using ivec3 = glm::ivec3;
using ivec4 = glm::ivec4;

using uvec3 = glm::uvec3;
using uvec4 = glm::uvec4;

using mat3 = glm::mat3;
using mat4 = glm::mat4;

using position = glm::vec3;
using rotation = glm::vec3;
using scale = glm::vec3;

using colour = glm::fvec4;

void create_test_pattern(flecs::iter it) {
    it.world().defer_suspend();

    constexpr size_t total_tiles = 10 * 10;
    it.world().dim(total_tiles);// pre-allocate memory for entities

    u32 quad_cnt_x = std::sqrt(total_tiles);
    u32 quad_cnt_y = std::sqrt(total_tiles);
    f32 quad_size = 0.25F;
    f32 total_width = quad_cnt_x * quad_size; // total width covered by the quads
    f32 total_height = quad_cnt_y * quad_size;// total height covered by the quads

    // Calculate the offsets needed to center the quads
    f32 offset_x = -total_width / 2.0F + quad_size / 2.0F;
    f32 offset_y = -total_height / 2.0F + quad_size / 2.0F;

    for (i32 x = 0; x < quad_cnt_x; x++) {
        for (i32 y = 0; y < quad_cnt_y; y++) {
            // Generate normalized values for x and y
            f32 norm_x = static_cast<f32>(x) / quad_cnt_x;// normalize to range [0.0, 1.0]
            f32 norm_y = static_cast<f32>(y) / quad_cnt_y;// normalize to range [0.0, 1.0]

            // Create an entity
            flecs::entity ent = it.world().entity();

            // Calculate the position, rotation, scale and colour of the quad
            position const pos = {offset_x + quad_size * x, offset_y + quad_size * y, 0.0F};
            scale const scl = {quad_size, quad_size, 0.0F};
            colour const col = {norm_x, norm_y, 1.0F - norm_x, 1.0F};

            // Add the components to the entity
            ent.set<position>(pos);
            ent.set<scale>(scl);
            ent.set<colour>(col);
        }
    }

    it.world().defer_resume();
}

struct vertex {
    fvec3 position = {};
    fvec4 colour = {};
};

struct render_data {
    vertex staged_vertices[65535] = {};
    u64 staged_vertex_count = 0;
};

static render_data s_data = {};

int main() {
    flecs::world ecs;

    ecs.system<>("create_test_pattern").no_readonly().kind(flecs::OnStart).iter(create_test_pattern);

    ecs.system<position, scale, colour>("Submit Quads").kind(flecs::OnUpdate).each([](flecs::entity e, position &pos, scale &scale, colour &colour) {
        (void) e;

        const float left = pos.x - scale.x / 2;
        const float right = pos.x + scale.x / 2;
        const float top = pos.y + scale.y / 2;
        const float bottom = pos.y - scale.y / 2;

        vertex vertices[6] = {};

        vertices[0] = {
                {left, bottom, 0.0F},
                {colour.r, colour.g, colour.b, colour.a}};
        vertices[1] = {
                {left, top, 0.0F},
                {colour.r, colour.g, colour.b, colour.a}};
        vertices[2] = {
                {right, bottom, 0.0F},
                {colour.r, colour.g, colour.b, colour.a}};
        vertices[3] = {
                {right, bottom, 0.0F},
                {colour.r, colour.g, colour.b, colour.a}};
        vertices[4] = {
                {left, top, 0.0F},
                {colour.r, colour.g, colour.b, colour.a}};
        vertices[5] = {
                {right, top, 0.0F},
                {colour.r, colour.g, colour.b, colour.a}};

        std::memcpy(s_data.staged_vertices + s_data.staged_vertex_count, vertices, sizeof(vertices));
        s_data.staged_vertex_count += 6;
    });

    ecs.progress();

    return 0;
}
