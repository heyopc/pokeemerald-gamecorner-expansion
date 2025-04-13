//#include "game_corner_blackjack.h"
#include "snake.h"
#include "global.h"
#include "malloc.h"
#include "battle.h"
#include "bg.h"
#include "coins.h"
#include "data.h"
#include "daycare.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "field_message_box.h"
#include "international_string_util.h"
#include "m4a.h"
#include "main.h"
#include "menu.h"
#include "menu_helpers.h"
#include "naming_screen.h"
#include "new_game.h"
#include "overworld.h"
#include "palette.h"
#include "palette_util.h"
#include "pokemon.h"
#include "pokedex.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trade.h"
#include "trainer_pokemon_sprites.h"
#include "tv.h"
#include "window.h"
#include "constants/coins.h"
#include "constants/flags.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/vars.h"
#include "scanline_effect.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "field_specials.h"

enum
{
    SNAKE_STATE_INIT,
	SNAKE_ONIX_CRY,
	SNAKE_START_TEXT,
	SNAKE_STATE_MOVEMENT_LOOP,
	SNAKE_STATE_PROCESS_INPUT,
	SNAKE_STATE_SPRITE_UPDATE,
	SNAKE_STATE_GAMEOVER_LOOP,
	SNAKE_STATE_GAMEOVER,
	SNAKE_STATE_START_EXIT,
	SNAKE_STATE_EXIT,
	SNAKE_EXIT,
};

enum {
    SPR_CREDIT_DIG_1,
    SPR_CREDIT_DIG_10,
    SPR_CREDIT_DIG_100,
    SPR_CREDIT_DIG_1000,
};

enum {
    SPR_NUM_DIG_1,
    SPR_NUM_DIG_10,
};

enum {
    SPR_BODY_1,
    SPR_BODY_2,
    SPR_BODY_3,
    SPR_BODY_4,
	SPR_BODY_5,
    SPR_BODY_6,
    SPR_BODY_7,
    SPR_BODY_8,
	SPR_BODY_9,
    SPR_BODY_10,
    SPR_BODY_11,
    SPR_BODY_12,
	SPR_BODY_13,
    SPR_BODY_14,
    SPR_BODY_15,
    SPR_BODY_16,
	SPR_BODY_17,
    SPR_BODY_18,
    SPR_BODY_19,
    SPR_BODY_20,
	SPR_BODY_21,
    SPR_BODY_22,
    SPR_BODY_23,
    SPR_BODY_24,
	SPR_BODY_25,
    SPR_BODY_26,
    SPR_BODY_27,
    SPR_BODY_28,
	SPR_BODY_29,
    SPR_BODY_30,
    SPR_BODY_31,
    SPR_BODY_32,
	SPR_BODY_33,
    SPR_BODY_34,
    SPR_BODY_35,
    SPR_BODY_36,
	SPR_BODY_37,
    SPR_BODY_38,
    SPR_BODY_39,
    SPR_BODY_40,
};

enum {
    X_BODY_1,
    X_BODY_2,
    X_BODY_3,
    X_BODY_4,
	X_BODY_5,
    X_BODY_6,
    X_BODY_7,
    X_BODY_8,
	X_BODY_9,
    X_BODY_10,
    X_BODY_11,
    X_BODY_12,
	X_BODY_13,
    X_BODY_14,
    X_BODY_15,
    X_BODY_16,
	X_BODY_17,
    X_BODY_18,
    X_BODY_19,
    X_BODY_20,
	X_BODY_21,
    X_BODY_22,
    X_BODY_23,
    X_BODY_24,
	X_BODY_25,
    X_BODY_26,
    X_BODY_27,
    X_BODY_28,
	X_BODY_29,
    X_BODY_30,
    X_BODY_31,
    X_BODY_32,
	X_BODY_33,
    X_BODY_34,
    X_BODY_35,
    X_BODY_36,
	X_BODY_37,
    X_BODY_38,
    X_BODY_39,
    X_BODY_40,
};

enum {
    Y_BODY_1,
    Y_BODY_2,
    Y_BODY_3,
    Y_BODY_4,
	Y_BODY_5,
    Y_BODY_6,
    Y_BODY_7,
    Y_BODY_8,
	Y_BODY_9,
    Y_BODY_10,
    Y_BODY_11,
    Y_BODY_12,
	Y_BODY_13,
    Y_BODY_14,
    Y_BODY_15,
    Y_BODY_16,
	Y_BODY_17,
    Y_BODY_18,
    Y_BODY_19,
    Y_BODY_20,
	Y_BODY_21,
    Y_BODY_22,
    Y_BODY_23,
    Y_BODY_24,
	Y_BODY_25,
    Y_BODY_26,
    Y_BODY_27,
    Y_BODY_28,
	Y_BODY_29,
    Y_BODY_30,
    Y_BODY_31,
    Y_BODY_32,
	Y_BODY_33,
    Y_BODY_34,
    Y_BODY_35,
    Y_BODY_36,
	Y_BODY_37,
    Y_BODY_38,
    Y_BODY_39,
    Y_BODY_40,
};

enum {
    DIR_BODY_1,
    DIR_BODY_2,
    DIR_BODY_3,
    DIR_BODY_4,
	DIR_BODY_5,
    DIR_BODY_6,
    DIR_BODY_7,
    DIR_BODY_8,
	DIR_BODY_9,
    DIR_BODY_10,
    DIR_BODY_11,
    DIR_BODY_12,
	DIR_BODY_13,
    DIR_BODY_14,
    DIR_BODY_15,
    DIR_BODY_16,
	DIR_BODY_17,
    DIR_BODY_18,
    DIR_BODY_19,
    DIR_BODY_20,
	DIR_BODY_21,
    DIR_BODY_22,
    DIR_BODY_23,
    DIR_BODY_24,
	DIR_BODY_25,
    DIR_BODY_26,
    DIR_BODY_27,
    DIR_BODY_28,
	DIR_BODY_29,
    DIR_BODY_30,
    DIR_BODY_31,
    DIR_BODY_32,
	DIR_BODY_33,
    DIR_BODY_34,
    DIR_BODY_35,
    DIR_BODY_36,
	DIR_BODY_37,
    DIR_BODY_38,
    DIR_BODY_39,
    DIR_BODY_40,
};

enum {
    DIR_LAST_BODY_1,
    DIR_LAST_BODY_2,
    DIR_LAST_BODY_3,
    DIR_LAST_BODY_4,
	DIR_LAST_BODY_5,
    DIR_LAST_BODY_6,
    DIR_LAST_BODY_7,
    DIR_LAST_BODY_8,
	DIR_LAST_BODY_9,
    DIR_LAST_BODY_10,
    DIR_LAST_BODY_11,
    DIR_LAST_BODY_12,
	DIR_LAST_BODY_13,
    DIR_LAST_BODY_14,
    DIR_LAST_BODY_15,
    DIR_LAST_BODY_16,
	DIR_LAST_BODY_17,
    DIR_LAST_BODY_18,
    DIR_LAST_BODY_19,
    DIR_LAST_BODY_20,
	DIR_LAST_BODY_21,
    DIR_LAST_BODY_22,
    DIR_LAST_BODY_23,
    DIR_LAST_BODY_24,
	DIR_LAST_BODY_25,
    DIR_LAST_BODY_26,
    DIR_LAST_BODY_27,
    DIR_LAST_BODY_28,
	DIR_LAST_BODY_29,
    DIR_LAST_BODY_30,
    DIR_LAST_BODY_31,
    DIR_LAST_BODY_32,
	DIR_LAST_BODY_33,
    DIR_LAST_BODY_34,
    DIR_LAST_BODY_35,
    DIR_LAST_BODY_36,
	DIR_LAST_BODY_37,
    DIR_LAST_BODY_38,
    DIR_LAST_BODY_39,
    DIR_LAST_BODY_40,
};

#define SPR_CREDIT_DIGITS SPR_CREDIT_DIG_1
#define SPR_NUM_DIGITS SPR_NUM_DIG_1
#define SPR_BODY SPR_BODY_1
#define X_BODY X_BODY_1
#define Y_BODY Y_BODY_1
#define DIR_BODY DIR_BODY_1
#define DIR_LAST_BODY DIR_LAST_BODY_1

#define MAX_SPRITES_CREDIT 4
#define MAX_SPRITES_NUM 2
#define MAX_SPRITES_BODY 40
#define MAX_X_BODY 40
#define MAX_Y_BODY 40
#define MAX_DIR_BODY 40
#define MAX_DIR_LAST_BODY 40

#define DOWN 0
#define UP 1
#define RIGHT 2
#define LEFT 3

struct Snake {
	u8 state;
	u8 CreditSpriteIds[MAX_SPRITES_CREDIT];
	u8 NumberSpriteIds[MAX_SPRITES_NUM];
	u8 ToggleButtons;
	u8 OnixSpriteId;
	u8 OnixTileX;
	u8 OnixTileY;
	u8 BodySprite1Id;
	u8 Body1x;
	u8 Body1y;
	u8 Body1Direction;
	u8 Body1LastDirection;
	u8 BodySprite2Id;
	u8 Body2x;
	u8 Body2y;
	u8 Body2Direction;
	u8 Body2LastDirection;
	u8 BodySpriteIds[MAX_SPRITES_BODY];
	u8 BodySpritesX[MAX_X_BODY];
	u8 BodySpritesY[MAX_Y_BODY];
	u8 BodySpritesDirection[MAX_DIR_BODY];
	u8 BodySpritesLastDirection[MAX_DIR_LAST_BODY];
	u8 BodyCount;
	u8 Direction; // 0 = Down, 1 = Up, 2 = Right, 3 = Left
	u8 LastDirection;
	u32 FrameCount;
	u32 delay;
	u8 CanMove;
	u8 GameOver;
	u8 Loop;
	u8 BerrySpriteId;
	u8 BerryX;
	u8 BerryY;
	u8 StartSpriteId;
	u8 GameOverSpriteId;
	u32 timer;
	u8 MenuSpriteId;
	u32 Total;
};	

static EWRAM_DATA struct Snake *sSnake = NULL;
static EWRAM_DATA u8 sTextWindowId = 0;

static void FadeToSnakeScreen(u8 taskId);
static void InitSnakeScreen(void);
static void DerbyVBlankCallback(void);

static const u8 sTestText[] = _("TEST");

// Backgound

static const u32 Snake_BG_Img[] = INCBIN_U32("graphics/snake/snake-bg.4bpp.lz");
static const u8 Snake_Tilemap[] = INCBIN_U8("graphics/snake/snake-bg.bin.lz");
static const u16 Snake_BG_Pal[] = INCBIN_U16("graphics/snake/bg.gbapal");

// Onix

static const u32 OnixGFX[] = INCBIN_U32("graphics/snake/onix-head.4bpp.lz");

static const u32 BodyGFX[] = INCBIN_U32("graphics/snake/onix-body.4bpp.lz");
static const u16 OnixPAL[] = INCBIN_U16("graphics/snake/onix.gbapal");

static const u32 Body2GFX[] = INCBIN_U32("graphics/snake/onix-body-2.4bpp.lz");
static const u32 Body3GFX[] = INCBIN_U32("graphics/snake/onix-body-3.4bpp.lz");
static const u32 Body4GFX[] = INCBIN_U32("graphics/snake/onix-body-4.4bpp.lz");

// Berries

static const u32 Berry1GFX[] = INCBIN_U32("graphics/snake/berry1.4bpp.lz");
static const u32 Berry2GFX[] = INCBIN_U32("graphics/snake/berry3.4bpp.lz");
static const u32 Berry3GFX[] = INCBIN_U32("graphics/snake/berry3.4bpp.lz");
static const u16 BerryPAL[] = INCBIN_U16("graphics/snake/berry.gbapal");

// Text

static const u32 StartGFX[] = INCBIN_U32("graphics/snake/start.4bpp.lz");
static const u32 GameOverGFX[] = INCBIN_U32("graphics/snake/gameover.4bpp.lz");
static const u16 TextPAL[] = INCBIN_U16("graphics/snake/text.gbapal");

// Menu

static const u32 MenuGFX[] = INCBIN_U32("graphics/snake/menu.4bpp.lz");
static const u16 MenuPAL[] = INCBIN_U16("graphics/snake/menu.gbapal");

// Numbers

static const u32 Digits_Gfx[] = INCBIN_U32("graphics/snake/digits.4bpp.lz");
static const u16 Digits_Pal[] = INCBIN_U16("graphics/snake/digits.gbapal");


#define SNAKE_BG_BASE 1
#define SNAKE_MENUS 0

static const struct BgTemplate sSnakeBGtemplates[] = {
    {
       .bg = SNAKE_BG_BASE,
       .charBaseIndex = 2,
       .mapBaseIndex = 31,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 3,
       .baseTile = 0
   },
   {
        .bg = SNAKE_MENUS,
        .charBaseIndex = 0,
        .mapBaseIndex = 0x17,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

static const struct WindowTemplate sSnakeWinTemplates[] = {
	{
        .bg = SNAKE_MENUS,
        .tilemapLeft = 6,
        .tilemapTop = 5,
        .width = 9,
        .height = 2,
        .paletteNum = 0xF,
        .baseBlock = 0x194,		
	},
    DUMMY_WIN_TEMPLATE,
};



#define GFX_ONIX 1
#define GFX_BODY_1 2
#define GFX_BERRY_1 3
#define GFX_BERRY_2 4
#define GFX_BERRY_3 5
#define GFX_BODY_2 6
#define GFX_BODY_3 7
#define GFX_BODY_4 8
#define GFX_START 9
#define GFX_GAMEOVER 10
#define GFX_MENU 11
#define GFX_CREDIT_DIGITS 12

#define PAL_ONIX 1
#define PAL_BERRY 2
#define PAL_TEXT 3
#define PAL_MENU 4
#define PAL_DIGITS 5

static const struct SpritePalette sSpritePalettes[] =
{
    { .data = OnixPAL,      .tag = PAL_ONIX },
	{ .data = BerryPAL,		.tag = PAL_BERRY },
	{ .data = TextPAL,		.tag = PAL_TEXT },
	{ .data = MenuPAL,		.tag = PAL_MENU },
	{ .data = Digits_Pal,	.tag = PAL_DIGITS },
    {}
};

static const struct CompressedSpriteSheet sSpriteSheets_CreditsInterface[] =
{
    {
        .data = Digits_Gfx,
        .size = 0x280,
        .tag = GFX_CREDIT_DIGITS
    },
    {}
};

static const struct OamData sOam_CreditDigit =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(8x16),
    .size = SPRITE_SIZE(8x16),
    .priority = 2,
};

static const struct SpriteTemplate sSpriteTemplate_CreditDigit =
{
    .tileTag = GFX_CREDIT_DIGITS,
    .paletteTag = PAL_DIGITS,
    .oam = &sOam_CreditDigit,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const struct CompressedSpriteSheet sSpriteSheet_Menu =
{
    .data = MenuGFX,
    .size = 0x400,
    .tag = GFX_MENU,
};

static const struct OamData sOamData_Menu =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Menu =
{
    .tileTag = GFX_MENU,
    .paletteTag = PAL_MENU,
    .oam = &sOamData_Menu,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Start =
{
    .data = StartGFX,
    .size = 0x800,
    .tag = GFX_START,
};

static const struct CompressedSpriteSheet sSpriteSheet_GameOver =
{
    .data = GameOverGFX,
    .size = 0x800,
    .tag = GFX_GAMEOVER,
};

static const struct OamData sOamData_Text =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const union AnimCmd sTitleAnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 10),
    ANIMCMD_FRAME(32, 10),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd *const sTitleAnimCmds[] = {
    sTitleAnimCmd_0,  // Looping animation
};

static const struct SpriteTemplate sSpriteTemplate_Start =
{
    .tileTag = GFX_START,
    .paletteTag = PAL_TEXT,
    .oam = &sOamData_Text,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_GameOver =
{
    .tileTag = GFX_GAMEOVER,
    .paletteTag = PAL_TEXT,
    .oam = &sOamData_Text,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Berry_1 =
{
    .data = Berry1GFX,
    .size = 0x100,
    .tag = GFX_BERRY_1,
};

static const struct CompressedSpriteSheet sSpriteSheet_Berry_2 =
{
    .data = Berry2GFX,
    .size = 0x100,
    .tag = GFX_BERRY_2,
};

static const struct CompressedSpriteSheet sSpriteSheet_Berry_3 =
{
    .data = Berry3GFX,
    .size = 0x100,
    .tag = GFX_BERRY_3,
};

static const struct OamData sOamData_Berry =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Berry_1 =
{
    .tileTag = GFX_BERRY_1,
    .paletteTag = PAL_BERRY,
    .oam = &sOamData_Berry,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Berry_2 =
{
    .tileTag = GFX_BERRY_2,
    .paletteTag = PAL_BERRY,
    .oam = &sOamData_Berry,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Berry_3 =
{
    .tileTag = GFX_BERRY_3,
    .paletteTag = PAL_BERRY,
    .oam = &sOamData_Berry,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Onix =
{
    .data = OnixGFX,
    .size = 0x800,
    .tag = GFX_ONIX,
};

static const struct OamData sOamData_Onix =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
	.tileNum = 0,
    .priority = 0,
};

static const union AnimCmd sOnixAnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(8, 15),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd sOnixAnimCmd_1[] = 
{
    ANIMCMD_FRAME(16, 15),
    ANIMCMD_FRAME(24, 15),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd sOnixAnimCmd_2[] = 
{
    ANIMCMD_FRAME(32, 15),
    ANIMCMD_FRAME(40, 15),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd sOnixAnimCmd_3[] = 
{
    ANIMCMD_FRAME(48, 15),
    ANIMCMD_FRAME(56, 15),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd *const sOnixAnimCmds[] = {
    sOnixAnimCmd_0,  // Down
	sOnixAnimCmd_1,  // Up
	sOnixAnimCmd_2,  // Right
	sOnixAnimCmd_3,  // Left
};

static const struct SpriteTemplate sSpriteTemplate_Onix =
{
    .tileTag = GFX_ONIX,
    .paletteTag = PAL_ONIX,
    .oam = &sOamData_Onix,
    .anims = sOnixAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Onix_Body =
{
    .data = BodyGFX,
    .size = 0x200,
    .tag = GFX_BODY_1,
};

static const struct CompressedSpriteSheet sSpriteSheet_Onix_Body_2 =
{
    .data = Body2GFX,
    .size = 0x200,
    .tag = GFX_BODY_2,
};

static const struct CompressedSpriteSheet sSpriteSheet_Onix_Body_3 =
{
    .data = Body3GFX,
    .size = 0x200,
    .tag = GFX_BODY_3,
};

static const struct CompressedSpriteSheet sSpriteSheet_Onix_Body_4 =
{
    .data = Body4GFX,
    .size = 0x200,
    .tag = GFX_BODY_4,
};

static const union AnimCmd sOnixAnimCmd2_0[] = 
{
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(8, 15),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd *const sOnixBAnimCmds[] = {
    sOnixAnimCmd2_0,
};

static const struct SpriteTemplate sSpriteTemplate_Onix_Body =
{
    .tileTag = GFX_BODY_1,
    .paletteTag = PAL_ONIX,
    .oam = &sOamData_Onix,
    .anims = sOnixBAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Onix_Body_4 =
{
    .tileTag = GFX_BODY_4,
    .paletteTag = PAL_ONIX,
    .oam = &sOamData_Onix,
    .anims = sOnixBAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Onix_Body_2 =
{
    .tileTag = GFX_BODY_2,
    .paletteTag = PAL_ONIX,
    .oam = &sOamData_Onix,
    .anims = sOnixBAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Onix_Body_3 =
{
    .tileTag = GFX_BODY_3,
    .paletteTag = PAL_ONIX,
    .oam = &sOamData_Onix,
    .anims = sOnixBAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

typedef struct  {
    int TileID;
	s16 xReal;
	s16 yReal;
	u8 y;
	u8 x;
} SnakeTiles;

#define MAX_X 8
#define MAX_Y 7
#define MAX_TILES 72

static const SnakeTiles SnakeTilesArray[] = {
	
    {0, 56, 16, 0, 0}, // Row 1
	{1, 72, 16, 0, 1},
	{2, 88, 16, 0, 2},
	{3, 104, 16, 0, 3},
	{4, 120, 16, 0, 4},
	{5, 136, 16, 0, 5},
	{6, 152, 16, 0, 6},
	{7, 168, 16, 0, 7},
	{8, 184, 16, 0, 8},
	
	{9, 56, 32, 1, 0}, // Row 2
	{10, 72, 32, 1, 1},
	{11, 88, 32, 1, 2},
	{12, 104, 32, 1, 3},
	{13, 120, 32, 1, 4},
	{14, 136, 32, 1, 5},
	{15, 152, 32, 1, 6},
	{16, 168, 32, 1, 7},
	{17, 184, 32, 1, 8},
	
	{18, 56, 48, 2, 0}, // Row 3
	{19, 72, 48, 2, 1},
	{20, 88, 48, 2, 2},
	{21, 104, 48, 2, 3},
	{22, 120, 48, 2, 4},
	{23, 136, 48, 2, 5},
	{24, 152, 48, 2, 6},
	{25, 168, 48, 2, 7},
	{26, 184, 48, 2, 8},

	{27, 56, 64, 3, 0}, // Row 4
	{28, 72, 64, 3, 1},
	{29, 88, 64, 3, 2},
	{30, 104, 64, 3, 3},
	{31, 120, 64, 3, 4},
	{32, 136, 64, 3, 5},
	{33, 152, 64, 3, 6},
	{34, 168, 64, 3, 7},
	{35, 184, 64, 3, 8},
	
	{36, 56, 80, 4, 0}, // Row 5
	{37, 72, 80, 4, 1},
	{38, 88, 80, 4, 2},
	{39, 104, 80, 4, 3},
	{40, 120, 80, 4, 4},
	{41, 136, 80, 4, 5},
	{42, 152, 80, 4, 6},
	{43, 168, 80, 4, 7},
	{44, 184, 80, 4, 8},
	
	{45, 56, 96, 5, 0}, // Row 6
	{46, 72, 96, 5, 1},
	{47, 88, 96, 5, 2},
	{48, 104, 96, 5, 3},
	{49, 120, 96, 5, 4},
	{50, 136, 96, 5, 5},
	{51, 152, 96, 5, 6},
	{52, 168, 96, 5, 7},
	{53, 184, 96, 5, 8},
	
	{54, 56, 112, 6, 0}, // Row 7
	{55, 72, 112, 6, 1},
	{56, 88, 112, 6, 2},
	{57, 104, 112, 6, 3},
	{58, 120, 112, 6, 4},
	{59, 136, 112, 6, 5},
	{60, 152, 112, 6, 6},
	{61, 168, 112, 6, 7},
	{62, 184, 112, 6, 8},
	
	{63, 56, 128, 7, 0}, // Row 8
	{64, 72, 128, 7, 1},
	{65, 88, 128, 7, 2},
	{66, 104, 128, 7, 3},
	{67, 120, 128, 7, 4},
	{68, 136, 128, 7, 5},
	{69, 152, 128, 7, 6},
	{70, 168, 128, 7, 7},
	{71, 184, 128, 7, 8},

};

void StartSnake(void)
{
	u8 taskId = 0;
    sSnake = AllocZeroed(sizeof(struct Snake));
    taskId = CreateTask(FadeToSnakeScreen, 0);
}

static void FadeToSnakeScreen(u8 taskId)
{
	switch (gTasks[taskId].data[0])
    {
    case 0:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].data[0]++;
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            SetMainCallback2(InitSnakeScreen);
            DestroyTask(taskId);
        }
        break;
    }
}

static void SnakeVBlankCallback(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void SnakeMainCallback(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
}

static void HandleInput(void)
{
	if (sSnake->ToggleButtons == 1) 
	{
		if (JOY_NEW(A_BUTTON))
		{

		}
		else if (JOY_NEW(B_BUTTON))
		{

		}
	}
}

static void ExitSnake(void)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        FREE_AND_SET_NULL(sSnake);
    }
}

static void StartExitSnake(void)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    sSnake->state = SNAKE_EXIT;
}

static void CreateOnix(void)
{
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Onix.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix.size;
		s.tag = GFX_ONIX;
		LoadSpriteSheet(&s);
	
	sSnake->OnixSpriteId = CreateSprite(&sSpriteTemplate_Onix, 120, 64, 0); // 5, 4
}

static void CreateBody(s16 x, s16 y)
{
	u8 Body;
	struct SpriteSheet s;
	
	Body = Random() % 4;
	
	if (Body == 0)
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body.size;
		s.tag = GFX_BODY_1;
		LoadSpriteSheet(&s);
	
		sSnake->BodySpriteIds[(sSnake->BodyCount - 3)] = CreateSprite(&sSpriteTemplate_Onix_Body, x, y, sSnake->BodyCount);
	}
	else if (Body == 1)
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body_2.size;
		s.tag = GFX_BODY_2;
		LoadSpriteSheet(&s);
	
		sSnake->BodySpriteIds[(sSnake->BodyCount - 3)] = CreateSprite(&sSpriteTemplate_Onix_Body_2, x, y, sSnake->BodyCount);
	}
	else if (Body == 2)
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body_3.size;
		s.tag = GFX_BODY_3;
		LoadSpriteSheet(&s);
	
		sSnake->BodySpriteIds[(sSnake->BodyCount - 3)] = CreateSprite(&sSpriteTemplate_Onix_Body_3, x, y, sSnake->BodyCount);
	}
	else
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body_4.size;
		s.tag = GFX_BODY_4;
		LoadSpriteSheet(&s);
	
		sSnake->BodySpriteIds[(sSnake->BodyCount - 3)] = CreateSprite(&sSpriteTemplate_Onix_Body_4, x, y, sSnake->BodyCount);
	}
}

static void CreateBody1(void)
{
	s16 x = 120;
	s16 y = 48;
	u8 Body;
	struct SpriteSheet s;
	
	Body = 3;
	
	if (Body == 0)
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body.size;
		s.tag = GFX_BODY_1;
		LoadSpriteSheet(&s);
	
		sSnake->BodySprite1Id = CreateSprite(&sSpriteTemplate_Onix_Body, x, y, 1);
	}
	else if (Body == 1)
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body_2.size;
		s.tag = GFX_BODY_2;
		LoadSpriteSheet(&s);
	
		sSnake->BodySprite1Id = CreateSprite(&sSpriteTemplate_Onix_Body_2, x, y, 1);
	}
	else if (Body == 2)
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body_3.size;
		s.tag = GFX_BODY_3;
		LoadSpriteSheet(&s);
	
		sSnake->BodySprite1Id = CreateSprite(&sSpriteTemplate_Onix_Body_3, x, y, 1);
	}
	else
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body_4.size;
		s.tag = GFX_BODY_4;
		LoadSpriteSheet(&s);
	
		sSnake->BodySprite1Id = CreateSprite(&sSpriteTemplate_Onix_Body_4, x, y, 1);
	}
}

static void CreateBody2(void)
{
	s16 x = 120;
	s16 y = 32;
	u8 Body;
	struct SpriteSheet s;
	
	Body = Random() % 4;
	
	if (Body == 0)
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body.size;
		s.tag = GFX_BODY_1;
		LoadSpriteSheet(&s);
	
		sSnake->BodySprite2Id = CreateSprite(&sSpriteTemplate_Onix_Body, x, y, 2);
	}
	else if (Body == 1)
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body_2.size;
		s.tag = GFX_BODY_2;
		LoadSpriteSheet(&s);
	
		sSnake->BodySprite2Id = CreateSprite(&sSpriteTemplate_Onix_Body_2, x, y, 2);
	}
	else if (Body == 2)
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body_3.size;
		s.tag = GFX_BODY_3;
		LoadSpriteSheet(&s);
	
		sSnake->BodySprite2Id = CreateSprite(&sSpriteTemplate_Onix_Body_3, x, y, 2);
	}
	else
	{
        LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Onix_Body_4.size;
		s.tag = GFX_BODY_4;
		LoadSpriteSheet(&s);
	
		sSnake->BodySprite2Id = CreateSprite(&sSpriteTemplate_Onix_Body_4, x, y, 2);
	}
}

#define MAX_FRAME_COUNT 8
#define MAX_DELAY 1

static void HandleMovement(void)
{
	
	u8 Count = sSnake->BodyCount - 3;
	int j;
	int i;
	
	if (sSnake->Direction == DOWN)
	{
		if (sSnake->OnixTileY < MAX_Y)
		{
			sSnake->CanMove = 0;
			gSprites[sSnake->OnixSpriteId].y += 2;
		}
		else
		{
			sSnake->CanMove = 1;
		}
	}
	else if (sSnake->Direction == UP)
	{
		if (sSnake->OnixTileY > 0)
		{	
			sSnake->CanMove = 0;
			gSprites[sSnake->OnixSpriteId].y -= 2;
		}
		else
		{
			sSnake->CanMove = 1;
		}
	}
	else if (sSnake->Direction == LEFT)
	{
		if (sSnake->OnixTileX > 0)
		{
			sSnake->CanMove = 0;
			gSprites[sSnake->OnixSpriteId].x -= 2;
		}
		else
		{
			sSnake->CanMove = 1;
		}
	}
	else if (sSnake->Direction == RIGHT)
	{
		if (sSnake->OnixTileX < MAX_X)
		{
			sSnake->CanMove = 0;
			gSprites[sSnake->OnixSpriteId].x += 2;
		}
		else
		{
			sSnake->CanMove = 1;
		}
	}
	
	if (sSnake->Body1Direction == DOWN)
	{
		if (sSnake->CanMove == 0)
		{
			gSprites[sSnake->BodySprite1Id].y += 2;
		}
	}
	else if (sSnake->Body1Direction == UP)
	{
		if (sSnake->CanMove == 0)
		{
			gSprites[sSnake->BodySprite1Id].y -= 2;
		}
	}
	else if (sSnake->Body1Direction == LEFT)
	{
		if (sSnake->CanMove == 0)
		{
			gSprites[sSnake->BodySprite1Id].x -= 2;
		}
	}
	else if (sSnake->Body1Direction == RIGHT)
	{
		if (sSnake->CanMove == 0)
		{
			gSprites[sSnake->BodySprite1Id].x += 2;
		}
	}
	
	if (sSnake->Body2Direction == DOWN)
	{
		if (sSnake->CanMove == 0)
		{
			gSprites[sSnake->BodySprite2Id].y += 2;
		}
	}
	else if (sSnake->Body2Direction == UP)
	{
		if (sSnake->CanMove == 0)
		{
			gSprites[sSnake->BodySprite2Id].y -= 2;
		}
	}
	else if (sSnake->Body2Direction == LEFT)
	{
		if (sSnake->CanMove == 0)
		{
			gSprites[sSnake->BodySprite2Id].x -= 2;
		}
	}
	else if (sSnake->Body2Direction == RIGHT)
	{
		if (sSnake->CanMove == 0)
		{
			gSprites[sSnake->BodySprite2Id].x += 2;
		}
	}
	
	for (i = Count, j = 0; i > 0; i--, j++) {
		if (sSnake->BodySpritesDirection[j] == DOWN)
		{
			if (sSnake->CanMove == 0)
			{
				gSprites[sSnake->BodySpriteIds[j]].y += 2;
			}
		}
		else if (sSnake->BodySpritesDirection[j] == UP)
		{
			if (sSnake->CanMove == 0)
			{
				gSprites[sSnake->BodySpriteIds[j]].y -= 2;
			}
		}
		else if (sSnake->BodySpritesDirection[j] == LEFT)
		{
			if (sSnake->CanMove == 0)
			{
				gSprites[sSnake->BodySpriteIds[j]].x -= 2;
			}
		}
		else if (sSnake->BodySpritesDirection[j] == RIGHT)
		{
			if (sSnake->CanMove == 0)
			{
				gSprites[sSnake->BodySpriteIds[j]].x += 2;
			}
		}
	}
	
}

static void HandleMovementMinus(u8 num)
{
	
	u8 Count = sSnake->BodyCount - 3;
	int j;
	int i;
	
	if (sSnake->Direction == DOWN)
	{
		gSprites[sSnake->OnixSpriteId].y += (2 * num);
	}
	else if (sSnake->Direction == UP)
	{
		gSprites[sSnake->OnixSpriteId].y -= (2 * num);
	}
	else if (sSnake->Direction == LEFT)
	{
		gSprites[sSnake->OnixSpriteId].x -= (2 * num);
	}
	else if (sSnake->Direction == RIGHT)
	{
		gSprites[sSnake->OnixSpriteId].x += (2 * num);
	}
	
	if (sSnake->Body1Direction == DOWN)
	{
		gSprites[sSnake->BodySprite1Id].y += (2 * num);
	}
	else if (sSnake->Body1Direction == UP)
	{
			gSprites[sSnake->BodySprite1Id].y -= (2 * num);
	}
	else if (sSnake->Body1Direction == LEFT)
	{
		gSprites[sSnake->BodySprite1Id].x -= (2 * num);
	}
	else if (sSnake->Body1Direction == RIGHT)
	{
			gSprites[sSnake->BodySprite1Id].x += (2 * num);
	}
	
	if (sSnake->Body2Direction == DOWN)
	{
		gSprites[sSnake->BodySprite2Id].y += (2 * num);
	}
	else if (sSnake->Body2Direction == UP)
	{
			gSprites[sSnake->BodySprite2Id].y -= (2 * num);
	}
	else if (sSnake->Body2Direction == LEFT)
	{
			gSprites[sSnake->BodySprite2Id].x -= (2 * num);
	}
	else if (sSnake->Body2Direction == RIGHT)
	{
			gSprites[sSnake->BodySprite2Id].x += (2 * num);
	}
	
	for (i = Count, j = 0; i > 0; i--, j++) {
		if (sSnake->BodySpritesDirection[j] == DOWN)
		{
				gSprites[sSnake->BodySpriteIds[j]].y += (2 * num);
		}
		else if (sSnake->BodySpritesDirection[j] == UP)
		{
				gSprites[sSnake->BodySpriteIds[j]].y -= (2 * num);
		}
		else if (sSnake->BodySpritesDirection[j] == LEFT)
		{
				gSprites[sSnake->BodySpriteIds[j]].x -= (2 * num);
		}
		else if (sSnake->BodySpritesDirection[j] == RIGHT)
		{
				gSprites[sSnake->BodySpriteIds[j]].x += (2 * num);
		}
	}
	
}

static void UpdateLocations(void)
{
	u8 Count = sSnake->BodyCount - 3;
	int i;
	int j;
	int k;
	
	for (i = 0; i < MAX_TILES; i++) {
				if ((SnakeTilesArray[i].xReal == gSprites[sSnake->OnixSpriteId].x) && (SnakeTilesArray[i].yReal == gSprites[sSnake->OnixSpriteId].y)) {
					sSnake->OnixTileX = SnakeTilesArray[i].x;
					sSnake->OnixTileY = SnakeTilesArray[i].y;
				}
			}
			
	for (i = 0; i < MAX_TILES; i++) {
				if ((SnakeTilesArray[i].xReal == gSprites[sSnake->BodySprite1Id].x) && (SnakeTilesArray[i].yReal == gSprites[sSnake->BodySprite1Id].y)) {
					sSnake->Body1x = SnakeTilesArray[i].x;
					sSnake->Body1y = SnakeTilesArray[i].y;
				}
			}	

	for (i = 0; i < MAX_TILES; i++) {
				if ((SnakeTilesArray[i].xReal == gSprites[sSnake->BodySprite2Id].x) && (SnakeTilesArray[i].yReal == gSprites[sSnake->BodySprite2Id].y)) {
					sSnake->Body2x = SnakeTilesArray[i].x;
					sSnake->Body2y = SnakeTilesArray[i].y;
				}
			}	

	for (k = Count, j = 0; k >= 0; k--, j++) {
		
		for (i = 0; i < MAX_TILES; i++) {
				if ((SnakeTilesArray[i].xReal == gSprites[sSnake->BodySpriteIds[j]].x) && (SnakeTilesArray[i].yReal == gSprites[sSnake->BodySpriteIds[j]].y)) {
					sSnake->BodySpritesX[j] = SnakeTilesArray[i].x;
					sSnake->BodySpritesY[j] = SnakeTilesArray[i].y;
				}
			}	
	}		
}

static void UpdateDirections(void)
{
    u8 Count = sSnake->BodyCount - 3;
    int j;

    for (j = Count; j >= 0; j--)
    {
        if (j > 0)
        {
            sSnake->BodySpritesDirection[j] = sSnake->BodySpritesLastDirection[j - 1];
        }
        else // j == 0, right next to Body2
        {
            sSnake->BodySpritesDirection[0] = sSnake->Body2LastDirection;
        }
    }

    sSnake->Body2Direction = sSnake->Body1LastDirection;
    sSnake->Body1Direction = sSnake->LastDirection;
}

static int CheckBoundaries(void)
{
	u8 Count = sSnake->BodyCount - 3;
	int j;
	int i;
	
	if ((sSnake->Direction == RIGHT) && (sSnake->OnixTileX == MAX_X))
	{
		return 1;
	}
	else if ((sSnake->Direction == LEFT) && (sSnake->OnixTileX == 0))
	{
		return 1;
	}
	else if ((sSnake->Direction == UP) && (sSnake->OnixTileY == 0))
	{
		return 1;
	}
	else if ((sSnake->Direction == DOWN) && (sSnake->OnixTileY == MAX_Y))
	{
		return 1;
	}
	
	if ((sSnake->Direction == RIGHT) && (sSnake->OnixTileX == sSnake->Body1x - 1) && (sSnake->OnixTileY == sSnake->Body1y))
	{
		return 1;
	}
	else if ((sSnake->Direction == LEFT) && (sSnake->OnixTileX == sSnake->Body1x + 1) && (sSnake->OnixTileY == sSnake->Body1y))
	{
		return 1;
	}
	else if ((sSnake->Direction == UP) && (sSnake->OnixTileY == sSnake->Body1y + 1) && (sSnake->OnixTileX == sSnake->Body1x))
	{
		return 1;
	}
	else if ((sSnake->Direction == DOWN) && (sSnake->OnixTileY == sSnake->Body1y - 1) && (sSnake->OnixTileX == sSnake->Body1x))
	{
		return 1;
	}
	
	if ((sSnake->Direction == RIGHT) && (sSnake->OnixTileX == sSnake->Body2x - 1) && (sSnake->OnixTileY == sSnake->Body2y))
	{
		return 1;
	}
	else if ((sSnake->Direction == LEFT) && (sSnake->OnixTileX == sSnake->Body2x + 1) && (sSnake->OnixTileY == sSnake->Body2y))
	{
		return 1;
	}
	else if ((sSnake->Direction == UP) && (sSnake->OnixTileY == sSnake->Body2y + 1) && (sSnake->OnixTileX == sSnake->Body2x))
	{
		return 1;
	}
	else if ((sSnake->Direction == DOWN) && (sSnake->OnixTileY == sSnake->Body2y - 1) && (sSnake->OnixTileX == sSnake->Body2x))
	{
		return 1;
	}
	
	for (i = Count, j = 0; i > 0; i--, j++) {
		if ((sSnake->Direction == RIGHT) && (sSnake->OnixTileX == sSnake->BodySpritesX[j] - 1) && (sSnake->OnixTileY == sSnake->BodySpritesY[j]))
		{
			return 1;
		}
		else if ((sSnake->Direction == LEFT) && (sSnake->OnixTileX == sSnake->BodySpritesX[j] + 1) && (sSnake->OnixTileY == sSnake->BodySpritesY[j]))
		{
			return 1;
		}
		else if ((sSnake->Direction == UP) && (sSnake->OnixTileY == sSnake->BodySpritesY[j] + 1) && (sSnake->OnixTileX == sSnake->BodySpritesX[j]))
		{
			return 1;
		}
		else if ((sSnake->Direction == DOWN) && (sSnake->OnixTileY == sSnake->BodySpritesY[j] - 1) && (sSnake->OnixTileX == sSnake->BodySpritesX[j]))
		{
			return 1;
		}
	}
	
	return 0;
}

#define TRUE 1
#define FALSE 0

static void CreateBerry(void)
{
	s16 xf;
	s16 yf;
	u8 Berry;
	u8 Count = sSnake->BodyCount - 3;
	int i;
	int j;
	struct SpriteSheet s;

    while (1)
	{
		u8 x;
		u8 y;
		u8 occupied = FALSE;

		i = Random() % MAX_TILES;
		x = SnakeTilesArray[i].x;
		y = SnakeTilesArray[i].y;

		if ((x == 0 && y == 0) ||
			(x == 1 && y == 0) ||
			(x == 0 && y == 1))
			continue;

		if ((sSnake->OnixTileX == x && sSnake->OnixTileY == y) ||
			(sSnake->Body1x == x && sSnake->Body1y == y) ||
			(sSnake->Body2x == x && sSnake->Body2y == y))
		{
			continue;
		}

		for (j = 0; j < Count; j++) {
			if (sSnake->BodySpritesX[j] == x && sSnake->BodySpritesY[j] == y) {
				occupied = TRUE;
				break;
			}
		}

		if (!occupied) {
			sSnake->BerryX = x;
			sSnake->BerryY = y;
			break;
		}
	}
	
	for (i = 0; i < MAX_TILES; i++) {
				if ((SnakeTilesArray[i].x == sSnake->BerryX) && (SnakeTilesArray[i].y == sSnake->BerryY)) {
					xf = SnakeTilesArray[i].xReal;
					yf = SnakeTilesArray[i].yReal;
				}
			}
			
	Berry = Random() % 3;
	
	if (Berry == 0)
	{
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Berry_1.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Berry_1.size;
		s.tag = GFX_BERRY_1;
		LoadSpriteSheet(&s);
	
		sSnake->BerrySpriteId = CreateSprite(&sSpriteTemplate_Berry_1, xf, yf, 45);
	}
	else if	(Berry == 1)
	{
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Berry_2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Berry_2.size;
		s.tag = GFX_BERRY_2;
		LoadSpriteSheet(&s);
	
		sSnake->BerrySpriteId = CreateSprite(&sSpriteTemplate_Berry_2, xf, yf, 45);
	}
	else
	{
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Berry_3.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Berry_3.size;
		s.tag = GFX_BERRY_3;
		LoadSpriteSheet(&s);
	
		sSnake->BerrySpriteId = CreateSprite(&sSpriteTemplate_Berry_3, xf, yf, 45);
	}
}

static void SetCreditDigits(u16 num)
{
    u8 i;
    u16 d = 1000; 

    for (i = 0; i < 4; i++)
    {
        u8 digit = num / d;

        gSprites[sSnake->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].invisible = FALSE;

        if (i == 0 && num < 1000) {
            digit = 0;  
        }

        gSprites[sSnake->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].oam.tileNum =
            gSprites[sSnake->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].sheetTileStart + (digit * 2);

        gSprites[sSnake->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].oam.priority = 0;

        num = num % d;
        d = d / 10;
    }
	
    BuildOamBuffer();
}

static void SetNumberDigits(u16 num)
{
    u8 i;
    u16 d = 10;  

    for (i = 0; i < 2; i++)  
    {
        u8 digit = num / d;

        gSprites[sSnake->NumberSpriteIds[i + SPR_NUM_DIGITS]].invisible = FALSE;

        if (i == 0 && num < 10) {
            digit = 0;  
        }

        gSprites[sSnake->NumberSpriteIds[i + SPR_NUM_DIGITS]].oam.tileNum =
            gSprites[sSnake->NumberSpriteIds[i + SPR_NUM_DIGITS]].sheetTileStart + (digit * 2);

        gSprites[sSnake->NumberSpriteIds[i + SPR_NUM_DIGITS]].oam.priority = 0;

        num = num % d;
        d = d / 10;
    }
	
    BuildOamBuffer();
}

static void CreateCreditSprites(void)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sSpriteSheets_CreditsInterface) - 1; i++)  
    {
        struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheets_CreditsInterface[i].data, gDecompressionBuffer);
        s.data = gDecompressionBuffer;
        s.size = sSpriteSheets_CreditsInterface[i].size;
        s.tag = sSpriteSheets_CreditsInterface[i].tag;
        LoadSpriteSheet(&s);
    }

    for (i = 0; i < 4; i++)
    {
        sSnake->CreditSpriteIds[i + SPR_CREDIT_DIGITS] = CreateSprite(&sSpriteTemplate_CreditDigit, i * 5 + 21, 14, 0);
		gSprites[sSnake->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].oam.priority = 0;
    }
}

static void CreateNumberSprites(void)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sSpriteSheets_CreditsInterface) - 1; i++)  
    {
        struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheets_CreditsInterface[i].data, gDecompressionBuffer);
        s.data = gDecompressionBuffer;
        s.size = sSpriteSheets_CreditsInterface[i].size;
        s.tag = sSpriteSheets_CreditsInterface[i].tag;
        LoadSpriteSheet(&s);
    }

    for (i = 0; i < 2; i++)
    {
        sSnake->NumberSpriteIds[i + SPR_NUM_DIGITS] = CreateSprite(&sSpriteTemplate_CreditDigit, i * 5 + 26, 30, 0);
		gSprites[sSnake->CreditSpriteIds[i + SPR_NUM_DIGITS]].oam.priority = 0;
    }
}

static void CheckBerry(void)
{
	u8 NewDirection;
	u8 Count = sSnake->BodyCount - 3;
	u8 index = Count; 
	u8 Body;
	s16 xf;
	s16 yf;
	int i;
	struct SpriteSheet s;
	
	if ((sSnake->OnixTileX == sSnake->BerryX) && (sSnake->OnixTileY == sSnake->BerryY))
	{
		PlaySE(SE_USE_ITEM);
		if (GetCoins() < 9998)
		{
			SetCoins(GetCoins() + 2);
			SetCreditDigits(GetCoins());
		}
		else
		{
			SetCoins(9999);
			SetCreditDigits(GetCoins());
		}
		if (sSnake->Total < 99)
		{
			sSnake->Total++;
			SetNumberDigits(sSnake->Total);
		}
		DestroySpriteAndFreeResources(&gSprites[sSnake->BerrySpriteId]);
		LoadSpritePalettes(sSpritePalettes);
		if ((Count < MAX_SPRITES_BODY) && (sSnake->BodyCount < MAX_SPRITES_BODY))
		{
			if (Count == 0) // New Body
			{
				NewDirection = sSnake->Body2LastDirection;
				
				if (NewDirection == UP)
				{
					Body = Random() % 4;
	
					if (Body == 0)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body.size;
						s.tag = GFX_BODY_1;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body, gSprites[sSnake->BodySprite2Id].x, gSprites[sSnake->BodySprite2Id].y + 16, sSnake->BodyCount);
					}
					else if (Body == 1)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_2.size;
						s.tag = GFX_BODY_2;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_2, gSprites[sSnake->BodySprite2Id].x, gSprites[sSnake->BodySprite2Id].y + 16, sSnake->BodyCount);
					}
					else if (Body == 2)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_3.size;
						s.tag = GFX_BODY_3;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_3, gSprites[sSnake->BodySprite2Id].x, gSprites[sSnake->BodySprite2Id].y + 16, sSnake->BodyCount);
					}
					else
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_4.size;
						s.tag = GFX_BODY_4;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_4, gSprites[sSnake->BodySprite2Id].x, gSprites[sSnake->BodySprite2Id].y + 16, sSnake->BodyCount);
					}
				}
				else if (NewDirection == DOWN)
				{
					Body = Random() % 4;
	
					if (Body == 0)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body.size;
						s.tag = GFX_BODY_1;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body, gSprites[sSnake->BodySprite2Id].x, gSprites[sSnake->BodySprite2Id].y - 16, sSnake->BodyCount);
					}
					else if (Body == 1)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_2.size;
						s.tag = GFX_BODY_2;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_2, gSprites[sSnake->BodySprite2Id].x, gSprites[sSnake->BodySprite2Id].y - 16, sSnake->BodyCount);
					}
					else if (Body == 2)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_3.size;
						s.tag = GFX_BODY_3;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_3, gSprites[sSnake->BodySprite2Id].x, gSprites[sSnake->BodySprite2Id].y - 16, sSnake->BodyCount);
					}
					else
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_4.size;
						s.tag = GFX_BODY_4;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_4, gSprites[sSnake->BodySprite2Id].x, gSprites[sSnake->BodySprite2Id].y - 16, sSnake->BodyCount);
					}
				}
				else if (NewDirection == LEFT)
				{
					Body = Random() % 4;
	
					if (Body == 0)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body.size;
						s.tag = GFX_BODY_1;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body, gSprites[sSnake->BodySprite2Id].x + 16, gSprites[sSnake->BodySprite2Id].y, sSnake->BodyCount);
					}
					else if (Body == 1)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_2.size;
						s.tag = GFX_BODY_2;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_2, gSprites[sSnake->BodySprite2Id].x + 16, gSprites[sSnake->BodySprite2Id].y, sSnake->BodyCount);
					}
					else if (Body == 2)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_3.size;
						s.tag = GFX_BODY_3;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_3, gSprites[sSnake->BodySprite2Id].x + 16, gSprites[sSnake->BodySprite2Id].y, sSnake->BodyCount);
					}
					else
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_4.size;
						s.tag = GFX_BODY_4;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_4, gSprites[sSnake->BodySprite2Id].x + 16, gSprites[sSnake->BodySprite2Id].y, sSnake->BodyCount);
					}
				}
				else if (NewDirection == RIGHT)
				{
					Body = Random() % 4;
	
					if (Body == 0)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body.size;
						s.tag = GFX_BODY_1;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body, gSprites[sSnake->BodySprite2Id].x - 16, gSprites[sSnake->BodySprite2Id].y, sSnake->BodyCount);
					}
					else if (Body == 1)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_2.size;
						s.tag = GFX_BODY_2;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_2, gSprites[sSnake->BodySprite2Id].x - 16, gSprites[sSnake->BodySprite2Id].y, sSnake->BodyCount);
					}
					else if (Body == 2)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_3.size;
						s.tag = GFX_BODY_3;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_3, gSprites[sSnake->BodySprite2Id].x - 16, gSprites[sSnake->BodySprite2Id].y, sSnake->BodyCount);
					}
					else
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_4.size;
						s.tag = GFX_BODY_4;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_4, gSprites[sSnake->BodySprite2Id].x - 16, gSprites[sSnake->BodySprite2Id].y, sSnake->BodyCount);
					}
				}
				
				NewDirection = sSnake->Body2LastDirection;
				sSnake->BodySpritesDirection[index] = NewDirection;
				if (sSnake->BodyCount < MAX_SPRITES_BODY) {
				sSnake->BodyCount++; }
			}
			else if (Count > 0)
			{
				NewDirection = sSnake->BodySpritesLastDirection[index - 1];

				if (NewDirection == UP)
				{
					Body = Random() % 4;
	
					if (Body == 0)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body.size;
						s.tag = GFX_BODY_1;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body, gSprites[sSnake->BodySpriteIds[index - 1]].x, gSprites[sSnake->BodySpriteIds[index - 1]].y + 16, sSnake->BodyCount);
					}
					else if (Body == 1)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_2.size;
						s.tag = GFX_BODY_2;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_2, gSprites[sSnake->BodySpriteIds[index - 1]].x, gSprites[sSnake->BodySpriteIds[index - 1]].y + 16, sSnake->BodyCount);
					}
					else if (Body == 2)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_3.size;
						s.tag = GFX_BODY_3;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_3, gSprites[sSnake->BodySpriteIds[index - 1]].x, gSprites[sSnake->BodySpriteIds[index - 1]].y + 16, sSnake->BodyCount);
					}
					else
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_4.size;
						s.tag = GFX_BODY_4;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_4, gSprites[sSnake->BodySpriteIds[index - 1]].x, gSprites[sSnake->BodySpriteIds[index - 1]].y + 16, sSnake->BodyCount);
					}
				}
				else if (NewDirection == DOWN)
				{
					Body = Random() % 4;
	
					if (Body == 0)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body.size;
						s.tag = GFX_BODY_1;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body, gSprites[sSnake->BodySpriteIds[index - 1]].x, gSprites[sSnake->BodySpriteIds[index - 1]].y - 16, sSnake->BodyCount);
					}
					else if (Body == 1)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_2.size;
						s.tag = GFX_BODY_2;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_2, gSprites[sSnake->BodySpriteIds[index - 1]].x, gSprites[sSnake->BodySpriteIds[index - 1]].y - 16, sSnake->BodyCount);
					}
					else if (Body == 2)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_3.size;
						s.tag = GFX_BODY_3;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_3, gSprites[sSnake->BodySpriteIds[index - 1]].x, gSprites[sSnake->BodySpriteIds[index - 1]].y - 16, sSnake->BodyCount);
					}
					else
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_4.size;
						s.tag = GFX_BODY_4;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_4, gSprites[sSnake->BodySpriteIds[index - 1]].x, gSprites[sSnake->BodySpriteIds[index - 1]].y - 16, sSnake->BodyCount);
					}
				}
				else if (NewDirection == LEFT)
				{
					Body = Random() % 4;
	
					if (Body == 0)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body.size;
						s.tag = GFX_BODY_1;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body, gSprites[sSnake->BodySpriteIds[index - 1]].x + 16, gSprites[sSnake->BodySpriteIds[index - 1]].y, sSnake->BodyCount);
					}
					else if (Body == 1)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_2.size;
						s.tag = GFX_BODY_2;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_2, gSprites[sSnake->BodySpriteIds[index - 1]].x + 16, gSprites[sSnake->BodySpriteIds[index - 1]].y, sSnake->BodyCount);
					}
					else if (Body == 2)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_3.size;
						s.tag = GFX_BODY_3;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_3, gSprites[sSnake->BodySpriteIds[index - 1]].x + 16, gSprites[sSnake->BodySpriteIds[index - 1]].y, sSnake->BodyCount);
					}
					else
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_4.size;
						s.tag = GFX_BODY_4;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_4, gSprites[sSnake->BodySpriteIds[index - 1]].x + 16, gSprites[sSnake->BodySpriteIds[index - 1]].y, sSnake->BodyCount);
					}
				}
				else if (NewDirection == RIGHT)
				{
					Body = Random() % 4;
	
					if (Body == 0)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body.size;
						s.tag = GFX_BODY_1;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body, gSprites[sSnake->BodySpriteIds[index - 1]].x - 16, gSprites[sSnake->BodySpriteIds[index - 1]].y, sSnake->BodyCount);
					}
					else if (Body == 1)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_2.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_2.size;
						s.tag = GFX_BODY_2;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_2, gSprites[sSnake->BodySpriteIds[index - 1]].x - 16, gSprites[sSnake->BodySpriteIds[index - 1]].y, sSnake->BodyCount);
					}
					else if (Body == 2)
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_3.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_3.size;
						s.tag = GFX_BODY_3;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_3, gSprites[sSnake->BodySpriteIds[index - 1]].x - 16, gSprites[sSnake->BodySpriteIds[index - 1]].y, sSnake->BodyCount);
					}
					else
					{
						LZ77UnCompWram(sSpriteSheet_Onix_Body_4.data, gDecompressionBuffer);
						s.data = gDecompressionBuffer;
						s.size = sSpriteSheet_Onix_Body_4.size;
						s.tag = GFX_BODY_4;
						LoadSpriteSheet(&s);
					
						sSnake->BodySpriteIds[index] = CreateSprite(&sSpriteTemplate_Onix_Body_4, gSprites[sSnake->BodySpriteIds[index - 1]].x - 16, gSprites[sSnake->BodySpriteIds[index - 1]].y, sSnake->BodyCount);
					}
				}

				NewDirection = sSnake->BodySpritesLastDirection[index - 1];
				sSnake->BodySpritesDirection[index] = NewDirection;
				if (sSnake->BodyCount < MAX_SPRITES_BODY) {
				sSnake->BodyCount++; }
			}
			
			
		}
		CreateBerry();
	}
}

static void CreateStartMenu(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Start.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Start.size;
		s.tag = GFX_START;
		LoadSpriteSheet(&s);
	
	sSnake->StartSpriteId = CreateSprite(&sSpriteTemplate_Start, 120, 120, 0); 
}

static void CreateGameOver(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_GameOver.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_GameOver.size;
		s.tag = GFX_GAMEOVER;
		LoadSpriteSheet(&s);
	
	sSnake->GameOverSpriteId = CreateSprite(&sSpriteTemplate_GameOver, 120, 80, 0); 
}

static void CreateMenu(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Menu.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Menu.size;
		s.tag = GFX_MENU;
		LoadSpriteSheet(&s);
	
	sSnake->MenuSpriteId = CreateSprite(&sSpriteTemplate_Menu, 40, 24, 1); 
}

static void SnakeMain(u8 taskId)
{
	u8 Count = sSnake->BodyCount - 3;
	int j;
	int i;
	
	switch (sSnake->state)
	{
		case SNAKE_STATE_INIT:
			if (!gPaletteFade.active)
			{
				PlayCry_Normal(SPECIES_ONIX, 0);
				sSnake->timer = 75;
				sSnake->state = SNAKE_ONIX_CRY;
			}
			break;	
		case SNAKE_ONIX_CRY:
			if (sSnake->timer == 0)
			{
				PlaySE(SE_POKENAV_ON);
				CreateStartMenu();
				sSnake->timer = 50;
				sSnake->state = SNAKE_START_TEXT;
			}
			else
			{
				sSnake->timer--;
			}
			break;
		case SNAKE_START_TEXT:
			if (sSnake->timer == 0)
			{
				DestroySpriteAndFreeResources(&gSprites[sSnake->StartSpriteId]);
				PlayBGM(MUS_RG_VICTORY_ROAD);
				sSnake->state = SNAKE_STATE_MOVEMENT_LOOP;
			}
			else
			{
				sSnake->timer--;
			}
			break;
		case SNAKE_STATE_MOVEMENT_LOOP:
			if (sSnake->delay == 0)
			{
				if (sSnake->FrameCount == 0)
				{
					sSnake->delay = 0;
					sSnake->LastDirection = sSnake->Direction;
					sSnake->Body1LastDirection = sSnake->Body1Direction;
					sSnake->Body2LastDirection = sSnake->Body2Direction;
					
					for (i = Count, j = 0; i > 0; i--, j++) {
						sSnake->BodySpritesLastDirection[j] = sSnake->BodySpritesDirection[j];
					}
					sSnake->state = SNAKE_STATE_PROCESS_INPUT;
				}
				else
				{
					sSnake->GameOver = CheckBoundaries();
					if (sSnake->GameOver == 1)
					{
						PlayBGM(MUS_NONE);
						sSnake->FrameCount = sSnake->Loop;
						sSnake->delay = MAX_DELAY;
						PlaySE(SE_WALL_HIT);
						sSnake->state = SNAKE_STATE_GAMEOVER_LOOP;
					}
					HandleMovement();
					sSnake->FrameCount--;
					sSnake->delay = MAX_DELAY;
				}
			}
			else
			{
				sSnake->delay--;
			}
			break;
		case SNAKE_STATE_PROCESS_INPUT:
			if (sSnake->delay == 0)
			{
				UpdateLocations();
				if (((JOY_HELD(DPAD_UP)) || (JOY_NEW(DPAD_UP))) && sSnake->LastDirection != DOWN)
				{
					sSnake->Direction = UP;
				}
				else if (((JOY_HELD(DPAD_DOWN)) || (JOY_NEW(DPAD_DOWN))) && sSnake->LastDirection != UP)
				{
					sSnake->Direction = DOWN;
				}
				else if (((JOY_HELD(DPAD_LEFT)) || (JOY_NEW(DPAD_LEFT)))  && sSnake->LastDirection != RIGHT)
				{
					sSnake->Direction = LEFT;
				}
				else if (((JOY_HELD(DPAD_RIGHT)) || (JOY_NEW(DPAD_RIGHT)))  && sSnake->LastDirection != LEFT)
				{
					sSnake->Direction = RIGHT;
				}
				sSnake->state = SNAKE_STATE_SPRITE_UPDATE;
			}
			else
			{
				UpdateLocations();
				if (((JOY_HELD(DPAD_UP)) || (JOY_NEW(DPAD_UP))) && sSnake->LastDirection != DOWN)
				{
					sSnake->Direction = UP;
				}
				else if (((JOY_HELD(DPAD_DOWN)) || (JOY_NEW(DPAD_DOWN))) && sSnake->LastDirection != UP)
				{
					sSnake->Direction = DOWN;
				}
				else if (((JOY_HELD(DPAD_LEFT)) || (JOY_NEW(DPAD_LEFT))) && sSnake->LastDirection != RIGHT)
				{
					sSnake->Direction = LEFT;
				}
				else if (((JOY_HELD(DPAD_RIGHT)) || (JOY_NEW(DPAD_RIGHT))) && sSnake->LastDirection != LEFT)
				{
					sSnake->Direction = RIGHT;
				}
				sSnake->delay--;
			}
			break;
		case SNAKE_STATE_SPRITE_UPDATE:
			if (sSnake->Direction == DOWN)
			{
				gSprites[sSnake->OnixSpriteId].animNum = 0;
			}
			else if (sSnake->Direction == UP)
			{
				gSprites[sSnake->OnixSpriteId].animNum = 1;
			}
			else if (sSnake->Direction == RIGHT)
			{
				gSprites[sSnake->OnixSpriteId].animNum = 2;
			}
			else if (sSnake->Direction == LEFT)
			{
				gSprites[sSnake->OnixSpriteId].animNum = 3;
			}
			
			UpdateDirections();
			CheckBerry();
			HandleMovement();
			sSnake->FrameCount = MAX_FRAME_COUNT - 1;
			sSnake->delay = MAX_DELAY;
			sSnake->state = SNAKE_STATE_MOVEMENT_LOOP;
			break;		
		case SNAKE_STATE_GAMEOVER_LOOP:
			if (sSnake->delay == 0)
			{
				if ((sSnake->FrameCount == 0) && (sSnake->Loop > 0))
				{
					HandleMovementMinus((0 - 1));
					sSnake->Loop--;
					sSnake->delay = MAX_DELAY;
				}
				else if (sSnake->Loop > 0)
				{
					HandleMovementMinus(1);
					sSnake->FrameCount--;
					sSnake->delay = MAX_DELAY;
				}
				else if (sSnake->Loop == 0)
				{
					sSnake->timer = 140;
					PlayFanfare(MUS_TOO_BAD);
					CreateGameOver();
					sSnake->state = SNAKE_STATE_GAMEOVER;
				}
			}
			else
			{
				sSnake->delay--;
			}
			break;
		case SNAKE_STATE_GAMEOVER:
			if (sSnake->timer == 0)
			{
				SetCoins(GetCoins() - (sSnake->Total + sSnake->Total));
				VarSet(VAR_FLIP_WINNINGS, (sSnake->Total + sSnake->Total));
				StartExitSnake();
			}
			else
			{
				sSnake->timer--;
			}
			break;
		case SNAKE_EXIT:
			ExitSnake();
			break;
	}
}

static void InitSnakeScreen(void)
{	
    SetVBlankCallback(NULL);
    ResetAllBgsCoordinates();
    ResetVramOamAndBgCntRegs();
    ResetBgsAndClearDma3BusyFlags(0);
    ResetTempTileDataBuffers();
	InitBgsFromTemplates(0, sSnakeBGtemplates, ARRAY_COUNT(sSnakeBGtemplates));
	SetBgTilemapBuffer(SNAKE_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
	DecompressAndLoadBgGfxUsingHeap(SNAKE_BG_BASE, Snake_BG_Img, 0x800, 0, 0);
	CopyToBgTilemapBuffer(SNAKE_BG_BASE, Snake_Tilemap, 0, 0);
	ResetPaletteFade();
	LoadPalette(Snake_BG_Pal, 0, sizeof(Snake_BG_Pal));
	ResetSpriteData();
    FreeAllSpritePalettes();
	LoadSpritePalettes(sSpritePalettes);
	CreateMenu();
	CreateCreditSprites();
	SetCreditDigits(GetCoins());
	CreateNumberSprites();
	SetNumberDigits(0);
	CreateOnix();
	CreateBody1();
	CreateBody2();
	
	sSnake->BodyCount = 3;
	sSnake->OnixTileX = 3;
	sSnake->OnixTileY = 4;
	sSnake->Body1x = 3;
	sSnake->Body1y = 3;
	sSnake->Body2x = 3;
	sSnake->Body2y = 2;
	sSnake->delay = MAX_DELAY;
	sSnake->FrameCount = MAX_FRAME_COUNT;
	sSnake->Direction = DOWN;
	sSnake->Body1Direction = DOWN;
	sSnake->Body2Direction = DOWN;
	sSnake->Loop = 2;
	
	CreateBerry();
	
	CopyBgTilemapBufferToVram(SNAKE_BG_BASE);
	//CopyBgTilemapBufferToVram(BLOCKSTACKER_TEXT_MENUS);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_BG2_ON);
    ShowBg(SNAKE_BG_BASE);
	//ShowBg(BLOCKSTACKER_TEXT_MENUS);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(SnakeVBlankCallback);
    SetMainCallback2(SnakeMainCallback);
    CreateTask(SnakeMain, 1);
}