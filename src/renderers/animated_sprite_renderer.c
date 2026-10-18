#include "renderer_utils.h"
#include "renderer.h"
#include "renderers/animated_sprite_renderer.h"
#include <wlr/util/log.h>

void init_animated_sprite_shader(animated_sprite_shader_t *shader){
    wlr_log(WLR_DEBUG, "Initializing shader");

    shader->base.program = create_shader_program_from_files("shaders/vertex.glsl","shaders/spriteAnimation.glsl");

    if (!shader->base.program) {
        wlr_log(WLR_ERROR, "Failed to create shader program");
        return;
    }

    wlr_log(WLR_DEBUG, "Shader program: %u", shader->base.program);

    shader->base.posLoc = glGetAttribLocation(shader->base.program, "aPos");
    shader->base.uvLoc = glGetAttribLocation(shader->base.program, "aUV");

    shader->base.projectionLoc = glGetUniformLocation(shader->base.program, "uProjection");
    shader->base.viewLoc = glGetUniformLocation(shader->base.program, "uView");
    shader->base.modelLoc = glGetUniformLocation(shader->base.program, "uModel");

    shader->loc_animationTexture = glGetUniformLocation(shader->base.program, "animationTexture");
    shader->loc_time = glGetUniformLocation(shader->base.program, "uTime");
    shader->loc_inverted = glGetUniformLocation(shader->base.program, "uInverted");

    shader->loc_row = glGetUniformLocation(shader->base.program, "uRow");
    shader->loc_rowCount = glGetUniformLocation(shader->base.program, "uRowCount");
    shader->loc_frameCount = glGetUniformLocation(shader->base.program, "uFrameCount");
    shader->loc_timePerFrame = glGetUniformLocation(shader->base.program, "uTimePerFrame");
    shader->loc_cellWidthUV = glGetUniformLocation(shader->base.program, "uCellWidthUV");


}

static void draw_animated_sprite(
    wayterra_renderer_t *r,
    animated_sprite_shader_t *shader,
    const transform_t *transform,
    const sprite_t *sprite,
    const animation_t *animation)
{
    if (sprite->texture == 0) {
        return;
    }

    // Model matrix
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

    // Sprite texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glUniform1i(shader->loc_animationTexture, 0);

    // Animation parameters
    glUniform1f(shader->loc_row, (GLfloat)animation->row);
    glUniform1f(shader->loc_rowCount, (GLfloat)animation->rowCount);
    glUniform1f(shader->loc_frameCount, (GLfloat)animation->frameCount);
    glUniform1f(
        shader->loc_timePerFrame,
        animation->timeToNextFrame
    );

    glUniform1i(
        shader->loc_inverted,
        sprite->inverted ? GL_TRUE : GL_FALSE
    );

    // Horizontal cell size in UV space
    float cellWidthUV =
        animation->sheetWidth > 0.0f
            ? animation->cellWidth / animation->sheetWidth
            : 1.0f;

    glUniform1f(shader->loc_cellWidthUV, cellWidthUV);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void animated_sprite_renderer_draw(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height)
{
    animated_sprite_shader_t *animatedShader = &r->animatedSpriteShader;

    glUseProgram(animatedShader->base.program);
    setup_vertex_attributes(r, &animatedShader->base);

    // Pass uTime
    glUniform1f(
            animatedShader->loc_time,
            (GLfloat)game->timer
            );

    // Projection matrix
    mat4 projection;

    glm_ortho(
        0.0f, (float)width,
        (float)height, 0.0f,
        -1.0f, 1.0f,
        projection
    );

    glUniformMatrix4fv(
        animatedShader->base.projectionLoc,
        1,
        GL_FALSE,
        (const GLfloat *)projection
    );

    // View matrix
    mat4 view;
    glm_mat4_identity(view);


    //TODO CHANGELATER added camera offset
    float camera_x_offset = 0.f;
    float camera_y_offset = 0.0f;

    vec3 camera_offset = {
        width / 2.0f - game->player->transform.x - camera_x_offset,
        height / 2.0f - game->player->transform.y - camera_y_offset,
        0.0f
    };

    glm_translate(view, camera_offset);


    glUniformMatrix4fv(
        animatedShader->base.viewLoc,
        1,
        GL_FALSE,
        (const GLfloat *)view
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Draw animated sprites
    for (size_t i = 0; i < game->animated_count; i++) {
        animated_sprite_t *object = &game->animated_objects[i];

        draw_animated_sprite(
                r,
                &r->animatedSpriteShader,
                &object->transform,
                &object->sprite,
                &object->animation
                );
    }
    glDisableVertexAttribArray(animatedShader->base.posLoc);

    glDisableVertexAttribArray(animatedShader->base.uvLoc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
