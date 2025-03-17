/* ENGINE COPYRIGHT
** Jo Sega Saturn Engine
** Copyright (c) 2012-2017, Johannes Fetz (johannesfetz@gmail.com)
** All rights reserved.
**
** GAME COPYRIGHT
** Samuli J‰‰skel‰inen
*/

#include <jo/jo.h>
#include "TestModel.h"
#include "Up.h"
#include "Right.h"
#include "Down.h"
#include "Left.h"
#include "A.h"
#include "B.h"
#include "C.h"
#include "X.h"
#include "Y.h"
#include "Z.h"

typedef struct RgbColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RgbColor;

typedef struct HsvColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

typedef struct Buttons
{
    unsigned char direction;
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char x;
    unsigned char y;
    unsigned char z;
} Buttons;

static jo_camera    cam;
static float        rotationX, rotationY;
static float        rotationMultiplierX, rotationMultiplierY;
static jo_sound     sfxScore1;
static jo_sound     sfxScore2;
static jo_font      *joFont;
static jo_color     joColor;
static RgbColor     rgbColor;
static HsvColor     hsvColor;
static HsvColor     tempColor;
static HsvColor     hsvColorBg;
static bool         gameStarted;
static Buttons      targetButtons;
static Buttons      playerButtons[JO_INPUT_MAX_DEVICE];
static unsigned int scores[JO_INPUT_MAX_DEVICE];
static bool multitap;


void RandomizeTargetButtons()
{
    targetButtons.direction = jo_random(5) - 1;

    targetButtons.a = jo_random(2) - 1;
    targetButtons.b = jo_random(2) - 1;
    targetButtons.c = jo_random(2) - 1;

    targetButtons.x = jo_random(2) - 1;
    targetButtons.y = jo_random(2) - 1;
    targetButtons.z = jo_random(2) - 1;
}

RgbColor HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

void SetMeshColors(jo_3d_mesh *mesh)
{
    for(unsigned int i = 0; i < jo_3d_get_mesh_polygon_count(mesh); ++i)
    {
        // Adjust hue based per quad on master hue
        tempColor.h = hsvColor.h;
        if(i % 2) tempColor.h += 64;
        else if(i % 3) tempColor.h -= 64;

        // Set mesh color
        rgbColor = HsvToRgb(tempColor);
        joColor = JO_COLOR_RGB(rgbColor.r, rgbColor.g, rgbColor.b);
        jo_3d_set_mesh_polygon_color(mesh, joColor, i);
    }
}

void my_draw(void)
{
    // Spin the model
    rotationX -= 0.1f * rotationMultiplierX;
    rotationY += 0.066f * rotationMultiplierY;

    // Animate master hue
    hsvColor.h++;

    // Colorize models
    SetMeshColors(&MeshUnnamed1);
    SetMeshColors(&MeshUp);
    SetMeshColors(&MeshRight);
    SetMeshColors(&MeshDown);
    SetMeshColors(&MeshLeft);
    SetMeshColors(&MeshA);
    SetMeshColors(&MeshB);
    SetMeshColors(&MeshC);
    SetMeshColors(&MeshX);
    SetMeshColors(&MeshY);
    SetMeshColors(&MeshZ);

    // Animate background color
    hsvColorBg.h++;
    rgbColor = HsvToRgb(hsvColorBg);
    joColor = JO_COLOR_RGB(rgbColor.r, rgbColor.g, rgbColor.b);

    // Set background color
    jo_clear_background(joColor);

    // Draw texts
    jo_font_printf(joFont, 24, 24, 0.9f, "SIIKASAURUS");
    jo_font_printf(joFont, 24, 200, 0.9f, "CHANGE SPEED WITH L/R");
    if(gameStarted)
    {
         for(int i = 0; i < JO_INPUT_MAX_DEVICE; ++i)
        {
            if(jo_is_input_available(i))
            {
                if(multitap)
                {
                    if(i < 6)
                    {
                        jo_font_printf(joFont, 24, 48 + i * 16, 0.9f, "P%i:%i", i + 1, scores[i]);
                    }
                    else
                    {
                        jo_font_printf(joFont, 240, 48 + i * 16 - 96, 0.9f, "P%i:%i", i + 1, scores[i]);
                    }
                }
                else
                {
                    if(i == 0)
                    {
                        jo_font_printf(joFont, 24, 48, 0.9f, "P1:%i", scores[i]);
                    }
                    else
                    {
                        jo_font_printf(joFont, 24, 48 + 16, 0.9f, "P2:%i", scores[i]);
                    }
                }
            }
        }
    }
    else // Game NOT started
    {
        jo_font_printf(joFont, 64, 150, 1.9f, "PRESS START");
    }

// DEBUG
/*
    jo_printf(1, 1, "%i", targetButtons.direction);
    jo_printf(1, 2, "%i", targetButtons.x);
    jo_printf(2, 2, "%i", targetButtons.y);
    jo_printf(3, 2, "%i", targetButtons.z);
    jo_printf(1, 3, "%i", targetButtons.a);
    jo_printf(2, 3, "%i", targetButtons.b);
    jo_printf(3, 3, "%i", targetButtons.c);

    for(int i = 0; i < JO_INPUT_MAX_DEVICE; ++i)
    {
        if(jo_is_input_available(i))
        {
            if(i < 6)
            {
                jo_printf(1, 1 + 4 + 4 * i, "%i", playerButtons[i].direction);
                jo_printf(1, 2 + 4 + 4 * i, "%i", playerButtons[i].x);
                jo_printf(2, 2 + 4 + 4 * i, "%i", playerButtons[i].y);
                jo_printf(3, 2 + 4 + 4 * i, "%i", playerButtons[i].z);
                jo_printf(1, 3 + 4 + 4 * i, "%i", playerButtons[i].a);
                jo_printf(2, 3 + 4 + 4 * i, "%i", playerButtons[i].b);
                jo_printf(3, 3 + 4 + 4 * i, "%i", playerButtons[i].c);
            }
            else
            {
                jo_printf(1 + 5, 1 + 4 + 4 * i - 24, "%i", playerButtons[i].direction);
                jo_printf(1 + 5, 2 + 4 + 4 * i - 24, "%i", playerButtons[i].x);
                jo_printf(2 + 5, 2 + 4 + 4 * i - 24, "%i", playerButtons[i].y);
                jo_printf(3 + 5, 2 + 4 + 4 * i - 24, "%i", playerButtons[i].z);
                jo_printf(1 + 5, 3 + 4 + 4 * i - 24, "%i", playerButtons[i].a);
                jo_printf(2 + 5, 3 + 4 + 4 * i - 24, "%i", playerButtons[i].b);
                jo_printf(3 + 5, 3 + 4 + 4 * i - 24, "%i", playerButtons[i].c);
            }
        }
    }
*/

    // Set camera
    jo_3d_camera_look_at(&cam);

    // Render models

    jo_3d_push_matrix();
    {
        jo_3d_rotate_matrix_rad_x(rotationX);
        jo_3d_rotate_matrix_rad_y(rotationY);
        display_testmodel_mesh();
    }
    jo_3d_pop_matrix();

    if(gameStarted)
    {
        if(targetButtons.direction == 1)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_up_mesh();
            }
            jo_3d_pop_matrix();
        }

        if(targetButtons.direction == 2)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_right_mesh();
            }
            jo_3d_pop_matrix();
        }

        if(targetButtons.direction == 3)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_down_mesh();
            }
            jo_3d_pop_matrix();
        }

        if(targetButtons.direction == 4)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_left_mesh();
            }
            jo_3d_pop_matrix();
        }

        if(targetButtons.a)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_a_mesh();
            }
            jo_3d_pop_matrix();
        }

        if(targetButtons.b)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_b_mesh();
            }
            jo_3d_pop_matrix();
        }

        if(targetButtons.c)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_c_mesh();
            }
            jo_3d_pop_matrix();
        }

        if(targetButtons.x)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_x_mesh();
            }
            jo_3d_pop_matrix();
        }

        if(targetButtons.y)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_y_mesh();
            }
            jo_3d_pop_matrix();
        }

        if(targetButtons.z)
        {
            jo_3d_push_matrix();
            {
                jo_3d_rotate_matrix_rad_x(rotationX);
                jo_3d_rotate_matrix_rad_y(rotationY);
                display_z_mesh();
            }
            jo_3d_pop_matrix();
        }
    }
}

bool AreButtonsEqual(Buttons buttons1, Buttons buttons2)
{
    if(buttons1.direction != buttons2.direction) return false;
    if(buttons1.a != buttons2.a) return false;
    if(buttons1.b != buttons2.b) return false;
    if(buttons1.c != buttons2.c) return false;
    if(buttons1.x != buttons2.x) return false;
    if(buttons1.y != buttons2.y) return false;
    if(buttons1.z != buttons2.z) return false;
    return true;
}

void my_gamepad(void)
{
    multitap = !(jo_is_input_available(0) && jo_is_input_available(6) && jo_get_input_count() == 2);

    for(int i = 0; i < JO_INPUT_MAX_DEVICE; ++i)
    {
        if(jo_is_input_available(i))
        {
            if (jo_is_input_key_pressed(i, JO_KEY_R))
            {
                rotationMultiplierX += 0.033f;
                rotationMultiplierY += 0.033f;
            }
            else if (jo_is_input_key_pressed(i, JO_KEY_L))
            {
                rotationMultiplierX -= 0.033f;
                rotationMultiplierY -= 0.033f;
            }

            if(gameStarted)
            {
                if (jo_is_input_key_pressed(i, JO_KEY_UP))
                    playerButtons[i].direction = 1;
                else if (jo_is_input_key_pressed(i, JO_KEY_RIGHT))
                    playerButtons[i].direction = 2;
                else if (jo_is_input_key_pressed(i, JO_KEY_DOWN))
                    playerButtons[i].direction = 3;
                else if (jo_is_input_key_pressed(i, JO_KEY_LEFT))
                    playerButtons[i].direction = 4;
                else
                    playerButtons[i].direction = 0;

                playerButtons[i].a = jo_is_input_key_pressed(i, JO_KEY_A);
                playerButtons[i].b = jo_is_input_key_pressed(i, JO_KEY_B);
                playerButtons[i].c = jo_is_input_key_pressed(i, JO_KEY_C);

                playerButtons[i].x = jo_is_input_key_pressed(i, JO_KEY_X);
                playerButtons[i].y = jo_is_input_key_pressed(i, JO_KEY_Y);
                playerButtons[i].z = jo_is_input_key_pressed(i, JO_KEY_Z);

                if(AreButtonsEqual(playerButtons[i], targetButtons))
                {
                    scores[i]++;
                    jo_audio_play_sound_on_channel(&sfxScore2, 0);
                    RandomizeTargetButtons();
                    return;
                }
            }
            else
            {
                // Wait until START is pressed to begin the game
                if (jo_is_input_key_down(i, JO_KEY_START))
                {
                    gameStarted = true;
                    jo_clear_screen();
                    jo_audio_play_sound_on_channel(&sfxScore2, 0);
                    jo_audio_play_cd_track(2, 2, true);
                    return;
                }
            }
        }
    }

    if(!gameStarted)
    {
        // Keep randomizing starting buttons until START is pressed
        RandomizeTargetButtons();
    }
}

void jo_main(void)
{
    gameStarted = false;
    jo_random_seed = 0;

    rotationX = 0.0f;
    rotationY = 0.0f;
    rotationMultiplierX = 0.6f;
    rotationMultiplierY = 0.6f;

    rgbColor.r = 0;
    rgbColor.g = 0;
    rgbColor.b = 0;

    hsvColor.h = 0;
    hsvColor.s = 255;
    hsvColor.v = 255;

    tempColor.h = 0;
    tempColor.s = 255;
    tempColor.v = 255;

    hsvColorBg.h = 128;
    hsvColorBg.s = 255;
    hsvColorBg.v = 255;

    for(int i = 0; i < JO_INPUT_MAX_DEVICE; ++i)
    {
        scores[i] = 0;
    }

	jo_core_init(JO_COLOR_Black);

	jo_audio_load_pcm("P1SCORE.PCM", JoSoundStereo8Bit, &sfxScore1);
	jo_audio_load_pcm("P2SCORE.PCM", JoSoundStereo8Bit, &sfxScore2);
	sfxScore1.sample_rate = 32000;
	sfxScore2.sample_rate = 32000;

	joFont = jo_font_load(JO_ROOT_DIR, "FONT.TGA", JO_COLOR_Green, 8, 8, 1, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:/");

    jo_3d_camera_init(&cam);
    jo_core_add_callback(my_gamepad);
	jo_core_add_callback(my_draw);
	jo_core_run();
}
