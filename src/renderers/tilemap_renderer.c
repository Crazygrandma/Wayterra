#include "renderer_utils.h"
#include "renderers/tilemap_renderer.h"
#include <wlr/util/log.h>




void init_tilemap_shader(tilemap_shader_t *shader){

    shader->base.program = create_shader_program_from_files("shaders/vertex.glsl","shaders/tilemap.glsl");

    if (!shader->base.program) {
        wlr_log(WLR_ERROR, "Failed to create shader program");
        return;
    }

    wlr_log(WLR_DEBUG, "Shader program: %u", shader->base.program);

    shader->base.posLoc = glGetAttribLocation(shader->base.program, "aPos");
    shader->base.uvLoc = glGetAttribLocation(shader->base.program, "aUV");
    
    shader->textureLoc = glGetUniformLocation(shader->base.program, "tilesetTexture");
    
    shader->base.projectionLoc = glGetUniformLocation(shader->base.program, "uProjection");
    shader->base.viewLoc = glGetUniformLocation(shader->base.program, "uView");
    shader->base.modelLoc = glGetUniformLocation(shader->base.program, "uModel");

    shader->tile_id_loc = glGetUniformLocation(shader->base.program, "tileId");
}


static void draw_tile(
        wayterra_renderer_t *r,
        tilemap_shader_t *shader,
        const transform_t *transform,
        const sprite_t *sprite,
        int tileId)
{
    if (sprite->texture == 0) {
        return;
    }

    // Model matrix: position and dimensions of the tile.
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

    // Select the tile within the tileset.
    glUniform1f(shader->tile_id_loc, (GLfloat)tileId);

    // Bind the tileset texture.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glUniform1i(shader->textureLoc, 0);

    // Draw the unit quad.
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void tilemap_renderer_draw(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height)
{
    tilemap_shader_t *shader = &r->tilemapShader;

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

    // View matrix: identity, the tilemap is drawn in screen space for now
    // and should not follow the camera.
    mat4 view;
    glm_mat4_identity(view);

    vec3 camera_offset = {
        width / 2.0f
            - game->player->transform.x
            - game->player->transform.width / 2.0f,

        height / 2.0f
            - game->player->transform.y
            - game->player->transform.height / 2.0f,

        0.0f
    };

    glm_translate(view, camera_offset);

    glUniformMatrix4fv(
        shader->base.viewLoc,
        1,
        GL_FALSE,
        (const GLfloat *)view
    );

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // The tiles form the background layer, so they must not write depth
    // (that would occlude everything drawn after them at the same z).
    glDisable(GL_DEPTH_TEST);

    // One draw call per tile.
    for (size_t i = 0; i < game->tile_count; i++) {
        tile_t *tile = &game->tiles[i];

        draw_tile(
                r,
                shader,
                &tile->transform,
                &tile->sprite,
                tile->id
                );
    }

    glEnable(GL_DEPTH_TEST);

    glDisableVertexAttribArray(shader->base.posLoc);

    glDisableVertexAttribArray(shader->base.uvLoc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
