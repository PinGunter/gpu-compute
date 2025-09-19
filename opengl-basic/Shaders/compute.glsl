#version 430 core

// Tamaño del grupo de trabajo local (16x16 hilos por grupo)
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// La textura de destino a la que escribiremos.
// 'rgba32f' significa que cada píxel tiene 4 componentes (RGBA) de 32-bit float.
// 'binding = 0' la asocia a la unidad de imagen 0.
layout (rgba32f, binding = 0) uniform writeonly image2D destTex;

// Un 'uniform' para animar el resultado basado en el tiempo
uniform float time;

void main() {
    // Coordenadas globales del píxel que este hilo está procesando
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(destTex);

    // Evitar escribir fuera de los límites de la textura
    if (pixel_coords.x >= size.x || pixel_coords.y >= size.y) {
        return;
    }

    // Normalizar las coordenadas (de 0.0 a 1.0)
    vec2 uv = vec2(pixel_coords) / vec2(size);

    // --- Lógica para generar un patrón dinámico (efecto lava) ---
    float r = 0.5 + 0.5 * sin(uv.x * 10.0 + time);
    float g = 0.5 + 0.5 * sin(uv.y * 10.0 + time * 0.8);
    float b = 0.5 + 0.5 * cos((uv.x + uv.y) * 10.0 + time * 0.5);

    vec4 color = vec4(r, g, b, 1.0);

    // Escribir el color calculado en la textura de destino
    imageStore(destTex, pixel_coords, color);
}