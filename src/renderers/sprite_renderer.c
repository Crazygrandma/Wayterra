#include "renderer_utils.h"
#include "renderers/sprite_renderer.h"
#include "game_scene.h"

#include <wlr/util/log.h>

void init_sprite_shader(sprite_shader_t *shader)
{
    shader->base.program = create_shader_program_from_files(
        "shaders/vertex.glsl",
        "shaders/sprite.glsl"
    );

    if (!shader->base.program) {
        wlr_log(WLR_ERROR, "Failed to create shader program");
        return;
    }

    wlr_log(WLR_DEBUG, "Shader program: %u", shader->base.program);

    shader->base.posLoc = glGetAttribLocation(shader->base.program, "aPos");

    shader->base.uvLoc = glGetAttribLocation(shader->base.program, "aUV");

    shader->textureLoc = glGetUniformLocation(shader->base.program, "spriteTexture");

    shader->base.projectionLoc = glGetUniformLocation(shader->base.program, "uProjection");

    shader->base.viewLoc = glGetUniformLocation(shader->base.program, "uView");

    shader->base.modelLoc = glGetUniformLocation(shader->base.program, "uModel");
}

static void draw_sprite(
    wayterra_renderer_t *r,
    sprite_shader_t *shader,
    const transform_t *transform,
    const sprite_t *sprite)
{
    if (sprite->texture == 0) {
        return;
    }

    mat4 model;
    glm_mat4_identity(model);

    vec3 position = {
        transform->x,
        transform->y,
        transform->z
    };

    vec3 scale = {
        transform->width,
        transform->height,
        1.0f
    };

    glm_translate(model, position);
    glm_scale(model, scale);

    glUniformMatrix4fv(
        shader->base.modelLoc,
        1,
        GL_FALSE,
        (const GLfloat *)model
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glUniform1i(shader->textureLoc, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void sprite_renderer_draw(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height)
{
    sprite_shader_t *shader = &r->spriteShader;

    glUseProgram(shader->base.program);
    setup_vertex_attributes(r, &shader->base);

    // Projection matrix
    mat4 projection;

    glm_ortho(
        0.0f, (float)width,
        (float)height, 0.0f,
        -1.0f, 1.0f,
        projection
    );

    glUniformMatrix4fv(
        shader->base.projectionLoc,
        1,
        GL_FALSE,
        (const GLfloat *)projection
    );

    // View matrix
    mat4 view;
    glm_mat4_identity(view);

    // TODO later add camera to scene with camera zoom
    float scale = 1.0f; // e.g. 1.0f, 2.0f, 0.5f

    // Zoom first, then translate. cglm post-multiplies, so the view
    // becomes S * T: the offset is applied in world (pre-zoom) units,
    // which is what the camera_offset formula below assumes. Scaling
    // after the translation (T * S) would divide the offset by `scale`
    // and push the player off center by that factor.
    vec3 zoom = {scale, scale, 1.0f};
    glm_scale(view, zoom);

    // Center the camera on the player.
    if (game->player != NULL) {
        vec3 camera_offset = {
            width / (2.0f * scale)
                - game->player->transform.x
                - game->player->transform.width / 2.0f,

            height / (2.0f * scale)
                - game->player->transform.y
                - game->player->transform.height / 2.0f,

            0.0f
        };

        glm_translate(view, camera_offset);
    }

    glUniformMatrix4fv(
            shader->base.viewLoc,
            1,
            GL_FALSE,
            (const GLfloat *)view
            );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render only regular sprite objects.
    for (size_t i = 0; i < game->scene.count; i++) {
        game_object_t *object = &game->scene.objects[i];

        if (object->type != GAME_OBJECT_SPRITE) {
            continue;
        }

        game_sprite_t *sprite_object = &object->data.sprite;

        draw_sprite(
            r,
            shader,
            &sprite_object->transform,
            &sprite_object->sprite
        );
    }

    glDisable(GL_BLEND);

    glDisableVertexAttribArray(shader->base.posLoc);
    glDisableVertexAttribArray(shader->base.uvLoc);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
