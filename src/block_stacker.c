#include "game_corner_block_stacker.h"
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

enum {
	STACKER_HIGHLIGHT,
	STACKER_HIGHLIGHT_END,
	STACKER_GO_DELAY,
	STACKER_GO,
	STACKER_LEVEL_SETUP,
	STACKER_INPUT,
	STACKER_CHECK_POS,
	STACKER_ROW_DELAY,
	STACKER_KEEP_GOING,
	STACKER_POST_DELAY,
	STACKER_WIN,
	STACKER_GAME_OVER,
	STACKER_START_EXIT,
	STACKER_EXIT,
};

enum {
	HighlightSpriteId_1a,
	HighlightSpriteId_2a,
	HighlightSpriteId_3a,
	HighlightSpriteId_4a,
	HighlightSpriteId_5a,
	HighlightSpriteId_6a,
	HighlightSpriteId_7a,
	HighlightSpriteId_1b,
	HighlightSpriteId_2b,
	HighlightSpriteId_3b,
	HighlightSpriteId_4b,
	HighlightSpriteId_5b,
	HighlightSpriteId_6b,
	HighlightSpriteId_7b,
};

enum {
    SPR_CREDIT_DIG_1,
    SPR_CREDIT_DIG_10,
    SPR_CREDIT_DIG_100,
    SPR_CREDIT_DIG_1000,
};

#define SPR_CREDIT_DIGITS SPR_CREDIT_DIG_1
#define SPR_HIGHLIGHT HighlightSpriteId_1a

#define MAX_SPRITES_HIGHLIGHT 14
#define MAX_SPRITES_CREDIT 4

struct BlockStacker {
	u8 state;
	u8 CreditSpriteIds[MAX_SPRITES_CREDIT];
	u8 RhydonSpriteId;
	u8 CommandsSpriteId;
	u8 ToggleButtons;
	u8 CurrentRow; // 1-8
	u8 Row1Block1Position; // 1-7
	u8 Row1Block2Position;
	u8 Row1Block3Position;
	u8 Row2Block1Position;
    u8 Row2Block2Position;
    u8 Row2Block3Position;
	u8 Row3Block1Position;
    u8 Row3Block2Position;
	u8 Row4Block1Position;
    u8 Row4Block2Position;
	u8 Row5BlockPosition;
	u8 Row6BlockPosition;
	u8 Row7BlockPosition;
	u8 Row8BlockPosition;
	u8 BlocksLeft; // 0-3
	u8 Row1Block1SpriteId;
    u8 Row1Block2SpriteId;
    u8 Row1Block3SpriteId;
    u8 Row2Block1SpriteId;
    u8 Row2Block2SpriteId;
    u8 Row2Block3SpriteId;
    u8 Row3Block1SpriteId;
    u8 Row3Block2SpriteId;
    u8 Row4Block1SpriteId;
    u8 Row4Block2SpriteId;
    u8 Row5Block1SpriteId;
    u8 Row6Block1SpriteId;
    u8 Row7Block1SpriteId;
    u8 Row8Block1SpriteId;
	u8 HighlightSpriteIds[MAX_SPRITES_HIGHLIGHT];
	u8 DestroyedHighlights;
	u8 HighlightNum;
	u8 HighlightRow;
	u8 TitleSpriteId;
	u16 GoDelay;
	u8 StartSpriteId;
	u8 ArrowSpriteId;
	u8 xSpeed;
	u8 xSpeedDelay;
	u8 xDirection;
	u8 exitToggle;
	u8 LastLives;
	u8 x1SpriteId;
	u8 x2SpriteId;
	u8 x3SpriteId;
	u8 x1Active;
	u8 x2Active;
	u8 x3Active;
	u8 GameOverSpriteId;
	u32 Winnings;
	u8 KeepGoingSpriteId;
	u8 YesSpriteId;
	u8 NoSpriteId;
	u8 YesNo;
	u8 WinnerSpriteId;
	u8 Win;
	u8 LivesSpriteId;
	u8 Rhydon2SpriteId;
	u8 RhydonBlockSpriteId;
};	

static EWRAM_DATA struct BlockStacker *sBlockStacker = NULL;
static EWRAM_DATA u8 sTextWindowId = 0;

static void FadeToBlockStackerScreen(u8 taskId);
static void InitBlockStackerScreen(void);
static void BlockStackerVBlankCallback(void);

// Backgound

static const u32 BlockStacker_BG_Img[] = INCBIN_U32("graphics/block_stacker/blockbgtiles.4bpp.lz");
static const u8 BlockStacker_Tilemap[] = INCBIN_U8("graphics/block_stacker/blockbgtiles.bin.lz");
static const u16 BlockStacker_BG_Pal[] = INCBIN_U16("graphics/block_stacker/bgblock.gbapal");

// Rhydon
static const u32 RhydonGFX[] = INCBIN_U32("graphics/block_stacker/rhydon.4bpp.lz");
static const u32 Rhydon2GFX[] = INCBIN_U32("graphics/block_stacker/rhydon2.4bpp.lz");
static const u16 RhydonPAL[] = INCBIN_U16("graphics/block_stacker/rhydon.gbapal");

static const u32 RhydonBlockGFX[] = INCBIN_U32("graphics/block_stacker/rhydonblock.4bpp.lz");
static const u16 RhydonBlockPAL[] = INCBIN_U16("graphics/block_stacker/rhydonblock.gbapal");

// Highlight
static const u32 HighlightGFX[] = INCBIN_U32("graphics/block_stacker/highlight.4bpp.lz");
static const u16 HighlightPAL[] = INCBIN_U16("graphics/block_stacker/highlight.gbapal");

// Title
static const u32 TitleGFX[] = INCBIN_U32("graphics/block_stacker/title.4bpp.lz");
static const u16 TitlePAL[] = INCBIN_U16("graphics/block_stacker/title.gbapal");

// Start
static const u32 StartGFX[] = INCBIN_U32("graphics/block_stacker/start.4bpp.lz");

// Commands
static const u32 CommandsGFX[] = INCBIN_U32("graphics/block_stacker/commands.4bpp.lz");
static const u16 CommandsPAL[] = INCBIN_U16("graphics/block_stacker/commands.gbapal");

// Arrow Icon
static const u32 ArrowGFX[] = INCBIN_U32("graphics/block_stacker/arrow.4bpp.lz");
static const u16 ArrowPAL[] = INCBIN_U16("graphics/block_stacker/arrow.gbapal");

// Blocks
static const u32 BlockGFX[] = INCBIN_U32("graphics/block_stacker/blocks.4bpp.lz");
static const u16 BlockPAL[] = INCBIN_U16("graphics/block_stacker/blocks.gbapal");

// X
static const u32 XGFX[] = INCBIN_U32("graphics/block_stacker/x.4bpp.lz");
static const u16 XPAL[] = INCBIN_U16("graphics/block_stacker/x.gbapal");

// Game Over
static const u32 GameOverGFX[] = INCBIN_U32("graphics/block_stacker/gameover.4bpp.lz");

// Keep Going
static const u32 KeepGoingGFX[] = INCBIN_U32("graphics/block_stacker/keepgoing.4bpp.lz");

// Yes / No
static const u32 YesGFX[] = INCBIN_U32("graphics/block_stacker/yes.4bpp.lz");
static const u32 NoGFX[] = INCBIN_U32("graphics/block_stacker/no.4bpp.lz");
static const u16 YesNoPAL[] = INCBIN_U16("graphics/block_stacker/yesno.gbapal");

// Winner
static const u32 WinnerGFX[] = INCBIN_U32("graphics/block_stacker/winner.4bpp.lz");

// Lives
static const u32 LivesGFX[] = INCBIN_U32("graphics/block_stacker/lives.4bpp.lz");
static const u16 LivesPAL[] = INCBIN_U16("graphics/block_stacker/lives.gbapal");

#define BLOCKSTACKER_BG 1
#define BLOCKSTACKER_TEXT_MENUS 2

static const struct BgTemplate sBlockStackerBGtemplates[] = {
    {
       .bg = BLOCKSTACKER_BG,
       .charBaseIndex = 2,
       .mapBaseIndex = 31,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 3,
       .baseTile = 0
   },
   {
        .bg = BLOCKSTACKER_TEXT_MENUS,
        .charBaseIndex = 0,
        .mapBaseIndex = 0x17,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

static const struct WindowTemplate sBlockStackerWinTemplates[] = {
	{
        .bg = BLOCKSTACKER_TEXT_MENUS,
        .tilemapLeft = 16,
        .tilemapTop = 9,
        .width = 14,
        .height = 2,
        .paletteNum = 0xF,
        .baseBlock = 0x194,		
	},
    DUMMY_WIN_TEMPLATE,
};

#define RHYDON_GFXTAG 1
#define HIGHLIGHT_GFXTAG 2
#define TITLE_GFXTAG 3
#define START_GFXTAG 4
#define COMMANDS_GFXTAG 5
#define ARROW_GFXTAG 6
#define BLOCK_GFXTAG 7
#define X_GFXTAG 8
#define GAMEOVER_GFXTAG 9
#define KEEPGOING_GFXTAG 10
#define YES_GFXTAG 11
#define NO_GFXTAG 12
#define WINNER_GFXTAG 13
#define LIVES_GFXTAG 14
#define RHYDON2_GFXTAG 15
#define RHYDONBLOCK_GFXTAG 16

#define RHYDON_PALTAG 1
#define HIGHLIGHT_PALTAG 2
#define TITLE_PALTAG 3
#define COMMANDS_PALTAG 4
#define ARROW_PALTAG 5
#define BLOCK_PALTAG 6
#define X_PALTAG 7
#define YESNO_PALTAG 8
#define LIVES_PALTAG 9
#define RHYDONBLOCK_PALTAG 10

static const struct SpritePalette sSpritePalettes[] =
{
    { .data = RhydonPAL,      .tag = RHYDON_PALTAG },
	{ .data = HighlightPAL,	  .tag = HIGHLIGHT_PALTAG },
	{ .data = TitlePAL,		  .tag = TITLE_PALTAG },
	{ .data = CommandsPAL,	  .tag = COMMANDS_PALTAG },
	{ .data = ArrowPAL,		  .tag = ARROW_PALTAG },
	{ .data = BlockPAL,		  .tag = BLOCK_PALTAG },
	{ .data = XPAL,			  .tag = X_PALTAG },
	{ .data = YesNoPAL,		  .tag = YESNO_PALTAG },
	{ .data = LivesPAL,		  .tag = LIVES_PALTAG },
	{ .data = RhydonBlockPAL, .tag = RHYDONBLOCK_PALTAG },
    {}
};

static const struct CompressedSpriteSheet sSpriteSheet_RhydonBlock =
{
    .data = RhydonBlockGFX,
    .size = 0x200,
    .tag = RHYDONBLOCK_GFXTAG,
};

static const struct OamData sOamData_RhydonBlock =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_RhydonBlock =
{
    .tileTag = RHYDONBLOCK_GFXTAG,
    .paletteTag = RHYDONBLOCK_PALTAG,
    .oam = &sOamData_RhydonBlock,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Lives =
{
    .data = LivesGFX,
    .size = 0xC0,
    .tag = LIVES_GFXTAG,
};

static const struct OamData sOamData_Lives =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x8),
    .size = SPRITE_SIZE(16x8),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Lives =
{
    .tileTag = LIVES_GFXTAG,
    .paletteTag = LIVES_PALTAG,
    .oam = &sOamData_Lives,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Yes =
{
    .data = YesGFX,
    .size = 0x600,
    .tag = YES_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_No =
{
    .data = NoGFX,
    .size = 0x600,
    .tag = NO_GFXTAG,
};

static const struct OamData sOamData_YesNo =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Yes =
{
    .tileTag = YES_GFXTAG,
    .paletteTag = YESNO_PALTAG,
    .oam = &sOamData_YesNo,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_No =
{
    .tileTag = NO_GFXTAG,
    .paletteTag = YESNO_PALTAG,
    .oam = &sOamData_YesNo,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_X =
{
    .data = XGFX,
    .size = 0x80,
    .tag = X_GFXTAG,
};

static const struct OamData sOamData_X =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_X =
{
    .tileTag = X_GFXTAG,
    .paletteTag = X_PALTAG,
    .oam = &sOamData_X,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Block =
{
    .data = BlockGFX,
    .size = 0x100,
    .tag = BLOCK_GFXTAG,
};

static const struct OamData sOamData_Block =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Block =
{
    .tileTag = BLOCK_GFXTAG,
    .paletteTag = BLOCK_PALTAG,
    .oam = &sOamData_Block,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Arrow =
{
    .data = ArrowGFX,
    .size = 0x200,
    .tag = ARROW_GFXTAG,
};

static const struct OamData sOamData_Arrow =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
	.tileNum = 0,
    .priority = 0,
};

static const union AnimCmd sArrowAnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 60),
    ANIMCMD_FRAME(4, 10),
	ANIMCMD_FRAME(8, 10),
	ANIMCMD_FRAME(12, 10),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd *const sArrowAnimCmds[] = {
    sArrowAnimCmd_0,  // Looping animation
};

static const struct SpriteTemplate sSpriteTemplate_Arrow =
{
    .tileTag = ARROW_GFXTAG,
    .paletteTag = ARROW_PALTAG,
    .oam = &sOamData_Arrow,
    .anims = sArrowAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Commands =
{
    .data = CommandsGFX,
    .size = 0x400,
    .tag = COMMANDS_GFXTAG,
};

static const struct OamData sOamData_Commands =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Commands =
{
    .tileTag = COMMANDS_GFXTAG,
    .paletteTag = COMMANDS_PALTAG,
    .oam = &sOamData_Commands,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Winner =
{
    .data = WinnerGFX,
    .size = 0x800,
    .tag = WINNER_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_KeepGoing =
{
    .data = KeepGoingGFX,
    .size = 0x800,
    .tag = KEEPGOING_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_GameOver =
{
    .data = GameOverGFX,
    .size = 0x800,
    .tag = GAMEOVER_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_Start =
{
    .data = StartGFX,
    .size = 0x800,
    .tag = START_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_Title =
{
    .data = TitleGFX,
    .size = 0x800,
    .tag = TITLE_GFXTAG,
};

static const struct OamData sOamData_Winner =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_KeepGoing =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_GameOver =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Start =
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

static const struct SpriteTemplate sSpriteTemplate_Winner =
{
    .tileTag = WINNER_GFXTAG,
    .paletteTag = TITLE_PALTAG,
    .oam = &sOamData_Winner,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_KeepGoing =
{
    .tileTag = KEEPGOING_GFXTAG,
    .paletteTag = TITLE_PALTAG,
    .oam = &sOamData_KeepGoing,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_GameOver =
{
    .tileTag = GAMEOVER_GFXTAG,
    .paletteTag = TITLE_PALTAG,
    .oam = &sOamData_GameOver,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Start =
{
    .tileTag = START_GFXTAG,
    .paletteTag = TITLE_PALTAG,
    .oam = &sOamData_Start,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct OamData sOamData_Title =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Title =
{
    .tileTag = TITLE_GFXTAG,
    .paletteTag = TITLE_PALTAG,
    .oam = &sOamData_Title,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Highlight =
{
    .data = HighlightGFX,
    .size = 0x280,
    .tag = HIGHLIGHT_GFXTAG,
};

static const struct OamData sOamData_Highlight =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
	.tileNum = 0,
    .priority = 0,
};

static const union AnimCmd sHighlightAnimCmd_0[] = 
{
	ANIMCMD_FRAME(16, 10),
	ANIMCMD_FRAME(12, 10),
	ANIMCMD_FRAME(8, 10),
	ANIMCMD_FRAME(4, 10),
	ANIMCMD_FRAME(0, 10),
    ANIMCMD_END
};

static const union AnimCmd sHighlightAnimCmd_1[] = 
{
	ANIMCMD_FRAME(0, 5),
    ANIMCMD_END
};

static const union AnimCmd sHighlightAnimCmd_2[] = 
{
	ANIMCMD_FRAME(0, 5),
	ANIMCMD_FRAME(4, 5),
	ANIMCMD_FRAME(8, 5),
	ANIMCMD_FRAME(12, 5),
	ANIMCMD_FRAME(16, 5),
    ANIMCMD_END
};

static const union AnimCmd *const sHighlightAnimCmds[] = {
    sHighlightAnimCmd_0, // Light Up
	sHighlightAnimCmd_1, // Still
	sHighlightAnimCmd_2, // Light Down
};

static const struct SpriteTemplate sSpriteTemplate_Highlight =
{
    .tileTag = HIGHLIGHT_GFXTAG,
    .paletteTag = HIGHLIGHT_PALTAG,
    .oam = &sOamData_Highlight,
    .anims = sHighlightAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Rhydon2 =
{
    .data = Rhydon2GFX,
    .size = 0x3800,
    .tag = RHYDON2_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_Rhydon =
{
    .data = RhydonGFX,
    .size = 0x4000,
    .tag = RHYDON_GFXTAG,
};

static const struct OamData sOamData_Rhydon2 =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Rhydon =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
	.tileNum = 0,
    .priority = 0,
};

static const union AnimCmd sRhydon2AnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 10),
    ANIMCMD_FRAME(64, 10),
    ANIMCMD_FRAME(128, 10),
    ANIMCMD_FRAME(192, 10),
    ANIMCMD_FRAME(256, 10),
    ANIMCMD_FRAME(320, 10),
	ANIMCMD_FRAME(384, 10),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd *const sRhydon2AnimCmds[] = {
    sRhydon2AnimCmd_0,  // Looping animation
};

static const union AnimCmd sRhydonAnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 10),
    ANIMCMD_FRAME(64, 10),
    ANIMCMD_FRAME(128, 10),
    ANIMCMD_FRAME(192, 10),
    ANIMCMD_FRAME(256, 10),
    ANIMCMD_FRAME(320, 10),
	ANIMCMD_FRAME(384, 10),
	ANIMCMD_FRAME(448, 10),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd *const sRhydonAnimCmds[] = {
    sRhydonAnimCmd_0,  // Looping animation
};

static const struct SpriteTemplate sSpriteTemplate_Rhydon2 =
{
    .tileTag = RHYDON2_GFXTAG,
    .paletteTag = RHYDON_PALTAG,
    .oam = &sOamData_Rhydon2,
    .anims = sRhydon2AnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Rhydon =
{
    .tileTag = RHYDON_GFXTAG,
    .paletteTag = RHYDON_PALTAG,
    .oam = &sOamData_Rhydon,
    .anims = sRhydonAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

void StartBlockStacker(void)
{
	u8 taskId = 0;
    sBlockStacker = AllocZeroed(sizeof(struct BlockStacker));
    taskId = CreateTask(FadeToBlockStackerScreen, 0);
}

static void FadeToBlockStackerScreen(u8 taskId)
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
            SetMainCallback2(InitBlockStackerScreen);
            DestroyTask(taskId);
        }
        break;
    }
}

static void BlockStackerVBlankCallback(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void BlockStackerMainCallback(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
}

static void CreateHighlight(u8 num, u8 row)
{
	if ((sBlockStacker->HighlightSpriteIds[num] == 0) && (sBlockStacker->HighlightRow != 8))
	{	
			struct SpriteSheet s;
			LoadSpritePalettes(sSpritePalettes);
			LZ77UnCompWram(sSpriteSheet_Highlight.data, gDecompressionBuffer);
			s.data = gDecompressionBuffer;
			s.size = sSpriteSheet_Highlight.size;
			s.tag = HIGHLIGHT_GFXTAG;
			LoadSpriteSheet(&s);
		if (num < 7)
		{
			sBlockStacker->HighlightSpriteIds[num] = CreateSprite(&sSpriteTemplate_Highlight, 32 + (16 * num), 136 - (16 * row), 1);
		}
		else
		{
			sBlockStacker->HighlightSpriteIds[num] = CreateSprite(&sSpriteTemplate_Highlight, ((32 + (16 * num)) - (16 * 7)), 136 - (16 * row), 1);
		}
		gSprites[sBlockStacker->HighlightSpriteIds[num]].animNum = 0; // Light Up
	}
	else if ((gSprites[sBlockStacker->HighlightSpriteIds[num]].animNum == 0) && (gSprites[sBlockStacker->HighlightSpriteIds[num]].animCmdIndex == 0))
	{
		gSprites[sBlockStacker->HighlightSpriteIds[num]].animNum = 2; // Light Down
		
		if ((sBlockStacker->HighlightNum != 6) && (sBlockStacker->HighlightNum != 13) && (sBlockStacker->HighlightRow != 8))
		{
			sBlockStacker->HighlightNum++; // 0-6
		}
		else if ((sBlockStacker->HighlightNum == 6) && (sBlockStacker->HighlightRow != 8))
		{
			sBlockStacker->HighlightNum ++; // 0-13
			sBlockStacker->HighlightRow++; // 0-7
		}
		else if ((sBlockStacker->HighlightNum == 13) && (sBlockStacker->HighlightRow != 8))
		{
			sBlockStacker->HighlightNum = 0; // 0-13
			sBlockStacker->HighlightRow++; // 0-7
		}
	}
}

static void DestroyHighlights(void)
{
	int i;
	
	for (i = 0; i < MAX_SPRITES_HIGHLIGHT; i++) {
		if ((gSprites[sBlockStacker->HighlightSpriteIds[i]].animNum == 2) && (gSprites[sBlockStacker->HighlightSpriteIds[i]].animCmdIndex > 3))
		{
			DestroySpriteAndFreeResources(&gSprites[sBlockStacker->HighlightSpriteIds[i]]);
			sBlockStacker->HighlightSpriteIds[i] = 0;
			sBlockStacker->DestroyedHighlights++;
		}
	}
}

static void SwapFromBlock(void)
{
		struct SpriteSheet s;
		DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Rhydon2SpriteId]);
		DestroySpriteAndFreeResources(&gSprites[sBlockStacker->RhydonBlockSpriteId]);
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Rhydon.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Rhydon.size;
		s.tag = RHYDON_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->RhydonSpriteId = CreateSprite(&sSpriteTemplate_Rhydon, 183, 112, 0);
}

static void SwapToBlock(void)
{
		struct SpriteSheet s;
		DestroySpriteAndFreeResources(&gSprites[sBlockStacker->RhydonSpriteId]);
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Rhydon2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Rhydon2.size;
		s.tag = RHYDON2_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->Rhydon2SpriteId = CreateSprite(&sSpriteTemplate_Rhydon2, 183, 112, 0);
	
		LZ77UnCompWram(sSpriteSheet_RhydonBlock.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_RhydonBlock.size;
		s.tag = RHYDONBLOCK_GFXTAG;
		LoadSpriteSheet(&s);
		
	sBlockStacker->RhydonBlockSpriteId = CreateSprite(&sSpriteTemplate_RhydonBlock, 177, 122, 0);
}

static void CreateRhydon(void)
{
		struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Rhydon.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Rhydon.size;
		s.tag = RHYDON_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->RhydonSpriteId = CreateSprite(&sSpriteTemplate_Rhydon, 183, 112, 0);
}

static void CreateArrow(void)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Arrow.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Arrow.size;
		s.tag = ARROW_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->ArrowSpriteId = CreateSprite(&sSpriteTemplate_Arrow, 14, 137, 0);
}

static void CreateCommands(void)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Commands.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Commands.size;
		s.tag = COMMANDS_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->CommandsSpriteId = CreateSprite(&sSpriteTemplate_Commands, 196, 50, 0);
}

static void CreateX1(s16 x, s16 y)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_X.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_X.size;
		s.tag = X_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->x1SpriteId = CreateSprite(&sSpriteTemplate_X, x, y, 0);
}

static void CreateX2(s16 x, s16 y)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_X.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_X.size;
		s.tag = X_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->x2SpriteId = CreateSprite(&sSpriteTemplate_X, x, y, 0);
}

static void CreateX3(s16 x, s16 y)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_X.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_X.size;
		s.tag = X_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->x3SpriteId = CreateSprite(&sSpriteTemplate_X, x, y, 0);
}

static void CreateYesNo(void)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Yes.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Yes.size;
		s.tag = YES_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->YesSpriteId = CreateSprite(&sSpriteTemplate_Yes, 50, 110, 0);
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_No.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_No.size;
		s.tag = NO_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->NoSpriteId = CreateSprite(&sSpriteTemplate_No, 104, 110, 0);
	gSprites[sBlockStacker->NoSpriteId].oam.tileNum += 32;
	
	sBlockStacker->YesNo = 0; // Yes
}

static void UpdateLives(void)
{
	if ((sBlockStacker->BlocksLeft == 2) && (sBlockStacker->LastLives == 3))
	{
		gSprites[sBlockStacker->LivesSpriteId].oam.tileNum -= 2;
	}
	else if ((sBlockStacker->BlocksLeft == 1) && (sBlockStacker->LastLives == 3))
	{
		gSprites[sBlockStacker->LivesSpriteId].oam.tileNum -= 4;
	}
	else if ((sBlockStacker->BlocksLeft == 1) && (sBlockStacker->LastLives == 2))
	{
		gSprites[sBlockStacker->LivesSpriteId].oam.tileNum -= 2;
	}
}

static void CreateLives(void)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Lives.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Lives.size;
		s.tag = LIVES_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->LivesSpriteId = CreateSprite(&sSpriteTemplate_Lives, 204, 24, 0);
	gSprites[sBlockStacker->LivesSpriteId].oam.tileNum += 4;
}

static void CreateKeepGoing(void)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_KeepGoing.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_KeepGoing.size;
		s.tag = KEEPGOING_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->KeepGoingSpriteId = CreateSprite(&sSpriteTemplate_KeepGoing, 80, 80, 0);
}

static void CreateGameOver(void)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_GameOver.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_GameOver.size;
		s.tag = GAMEOVER_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->GameOverSpriteId = CreateSprite(&sSpriteTemplate_GameOver, 80, 80, 0);
}

static void CreateStart(void)
{
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Start.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Start.size;
		s.tag = START_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->StartSpriteId = CreateSprite(&sSpriteTemplate_Start, 80, 80, 0);
}

static void DestroyKeepGoing(void)
{
	gSprites[sBlockStacker->KeepGoingSpriteId].invisible = TRUE;
	DestroySpriteAndFreeResources(&gSprites[sBlockStacker->KeepGoingSpriteId]);
}

static void DestroyStart(void)
{
	gSprites[sBlockStacker->StartSpriteId].invisible = TRUE;
	DestroySpriteAndFreeResources(&gSprites[sBlockStacker->StartSpriteId]);
}

static void DestroyLives(void)
{
	gSprites[sBlockStacker->LivesSpriteId].invisible = TRUE;
	DestroySpriteAndFreeResources(&gSprites[sBlockStacker->LivesSpriteId]);
}

static void CreateWinner(void)
{
		struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Winner.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Winner.size;
		s.tag = WINNER_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->WinnerSpriteId = CreateSprite(&sSpriteTemplate_Winner, 80, 80, 0);
}

static void CreateTitle(void)
{
		struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Title.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Title.size;
		s.tag = TITLE_GFXTAG;
		LoadSpriteSheet(&s);
	
	sBlockStacker->TitleSpriteId = CreateSprite(&sSpriteTemplate_Title, 80, 80, 0);
}

static void DestroyTitle(void)
{
	DestroySpriteAndFreeResources(&gSprites[sBlockStacker->TitleSpriteId]);
}

static void CreateLevel_1(void)
{
	u8 LR;
	
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Block.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Block.size;
		s.tag = BLOCK_GFXTAG;
		LoadSpriteSheet(&s);
	
	LR = (Random() % 100);
	
	if (LR < 50) // Right Side
	{
	sBlockStacker->Row1Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96, 136, 1);
	sBlockStacker->Row1Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 16, 136, 1);
	sBlockStacker->Row1Block3SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 136, 1);
	
	sBlockStacker->xDirection = 1; // Move Left
	}
	else // Left Side
	{
	sBlockStacker->Row1Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32, 136, 1);
	sBlockStacker->Row1Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 16, 136, 1);
	sBlockStacker->Row1Block3SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 136, 1);
	
	sBlockStacker->xDirection = 0; // Move Right
	}	
	
	//gSprites[sBlockStacker->Row1Block1SpriteId].oam.tileNum += 16;
	
	sBlockStacker->xSpeed = 10;
	sBlockStacker->xSpeedDelay = 0;
}

static void CreateLevel_2(void)
{
	u8 LR;
	
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Block.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Block.size;
		s.tag = BLOCK_GFXTAG;
		LoadSpriteSheet(&s);
	
	LR = (Random() % 100);
	
	if (sBlockStacker->BlocksLeft == 3)
	{
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row2Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row2Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 16, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row2Block3SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row2Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row2Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 16, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row2Block3SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	}
	else if (sBlockStacker->BlocksLeft == 2)
	{
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row2Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 16, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row2Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row2Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 16, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row2Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	}
	else if (sBlockStacker->BlocksLeft == 1)
	{
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row2Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row2Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	}
	
	//gSprites[sBlockStacker->Row1Block1SpriteId].oam.tileNum += 16;
	
	sBlockStacker->xSpeed = 9;
	sBlockStacker->xSpeedDelay = 0;
}

static void CreateLevel_3(void)
{
	u8 LR;
	
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Block.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Block.size;
		s.tag = BLOCK_GFXTAG;
		LoadSpriteSheet(&s);
	
	LR = (Random() % 100);
	
	if (sBlockStacker->BlocksLeft == 2)
	{
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row3Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 16, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row3Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row3Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 16, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row3Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	}
	else if (sBlockStacker->BlocksLeft == 1)
	{
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row3Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row3Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	}
	
	//gSprites[sBlockStacker->Row1Block1SpriteId].oam.tileNum += 16;
	
	sBlockStacker->xSpeed = 8;
	sBlockStacker->xSpeedDelay = 0;
}

static void CreateLevel_4(void)
{
	u8 LR;
	
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Block.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Block.size;
		s.tag = BLOCK_GFXTAG;
		LoadSpriteSheet(&s);
	
	LR = (Random() % 100);
	
	if (sBlockStacker->BlocksLeft == 2)
	{
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row4Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 16, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row4Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row4Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 16, 152 - (16 * sBlockStacker->CurrentRow), 1);
		sBlockStacker->Row4Block2SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	}
	else if (sBlockStacker->BlocksLeft == 1)
	{
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row4Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row4Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	}
	
	//gSprites[sBlockStacker->Row1Block1SpriteId].oam.tileNum += 16;
	
	sBlockStacker->xSpeed = 7;
	sBlockStacker->xSpeedDelay = 0;
}

static void CreateLevel_5(void)
{
	u8 LR;
	
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Block.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Block.size;
		s.tag = BLOCK_GFXTAG;
		LoadSpriteSheet(&s);
	
	LR = (Random() % 100);
	
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row5Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row5Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	
	//gSprites[sBlockStacker->Row1Block1SpriteId].oam.tileNum += 16;
	
	sBlockStacker->xSpeed = 5;
	sBlockStacker->xSpeedDelay = 0;
}

static void CreateLevel_6(void)
{
	u8 LR;
	
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Block.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Block.size;
		s.tag = BLOCK_GFXTAG;
		LoadSpriteSheet(&s);
	
	LR = (Random() % 100);
	
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row6Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row6Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	
	//gSprites[sBlockStacker->Row1Block1SpriteId].oam.tileNum += 16;
	
	sBlockStacker->xSpeed = 4;
	sBlockStacker->xSpeedDelay = 0;
}

static void CreateLevel_7(void)
{
	u8 LR;
	
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Block.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Block.size;
		s.tag = BLOCK_GFXTAG;
		LoadSpriteSheet(&s);
	
	LR = (Random() % 100);
	
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row7Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row7Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	
	//gSprites[sBlockStacker->Row1Block1SpriteId].oam.tileNum += 16;
	
	sBlockStacker->xSpeed = 3;
	sBlockStacker->xSpeedDelay = 0;
}

static void CreateLevel_8(void)
{
	u8 LR;
	
		struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Block.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Block.size;
		s.tag = BLOCK_GFXTAG;
		LoadSpriteSheet(&s);
	
	LR = (Random() % 100);
	
		if (LR < 50) // Right Side
		{
		sBlockStacker->Row8Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 96 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 1; // Move Left
		}
		else // Left Side
		{
		sBlockStacker->Row8Block1SpriteId = CreateSprite(&sSpriteTemplate_Block, 32 + 32, 152 - (16 * sBlockStacker->CurrentRow), 1);
		
		sBlockStacker->xDirection = 0; // Move Right
		}	
	
	//gSprites[sBlockStacker->Row1Block1SpriteId].oam.tileNum += 16;
	
	sBlockStacker->xSpeed = 2;
	sBlockStacker->xSpeedDelay = 0;
}

static void UpdateBlockPosition(void)
{
	if (sBlockStacker->xSpeedDelay == 0)
	{
		sBlockStacker->xSpeedDelay = sBlockStacker->xSpeed;
		
		// Per Level and Lives
		
		if ((sBlockStacker->CurrentRow == 1) && (sBlockStacker->BlocksLeft == 3)) // Level 1, 3 Lives
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row1Block3SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row1Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row1Block3SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row1Block1SpriteId].x = (gSprites[sBlockStacker->Row1Block1SpriteId].x + 16);
				gSprites[sBlockStacker->Row1Block2SpriteId].x = (gSprites[sBlockStacker->Row1Block2SpriteId].x + 16);
				gSprites[sBlockStacker->Row1Block3SpriteId].x = (gSprites[sBlockStacker->Row1Block3SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row1Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row1Block1SpriteId].x = (gSprites[sBlockStacker->Row1Block1SpriteId].x - 16);
				gSprites[sBlockStacker->Row1Block2SpriteId].x = (gSprites[sBlockStacker->Row1Block2SpriteId].x - 16);
				gSprites[sBlockStacker->Row1Block3SpriteId].x = (gSprites[sBlockStacker->Row1Block3SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 2) && (sBlockStacker->BlocksLeft == 3)) // Level 2, 3 Lives
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row2Block3SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row2Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row2Block3SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row2Block1SpriteId].x = (gSprites[sBlockStacker->Row2Block1SpriteId].x + 16);
				gSprites[sBlockStacker->Row2Block2SpriteId].x = (gSprites[sBlockStacker->Row2Block2SpriteId].x + 16);
				gSprites[sBlockStacker->Row2Block3SpriteId].x = (gSprites[sBlockStacker->Row2Block3SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row2Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row2Block1SpriteId].x = (gSprites[sBlockStacker->Row2Block1SpriteId].x - 16);
				gSprites[sBlockStacker->Row2Block2SpriteId].x = (gSprites[sBlockStacker->Row2Block2SpriteId].x - 16);
				gSprites[sBlockStacker->Row2Block3SpriteId].x = (gSprites[sBlockStacker->Row2Block3SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 2) && (sBlockStacker->BlocksLeft == 2)) // Level 2, 2 Lives
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row2Block2SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row2Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row2Block2SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row2Block1SpriteId].x = (gSprites[sBlockStacker->Row2Block1SpriteId].x + 16);
				gSprites[sBlockStacker->Row2Block2SpriteId].x = (gSprites[sBlockStacker->Row2Block2SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row2Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row2Block1SpriteId].x = (gSprites[sBlockStacker->Row2Block1SpriteId].x - 16);
				gSprites[sBlockStacker->Row2Block2SpriteId].x = (gSprites[sBlockStacker->Row2Block2SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 2) && (sBlockStacker->BlocksLeft == 1)) // Level 2, 1 Live
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row2Block1SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row2Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row2Block1SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row2Block1SpriteId].x = (gSprites[sBlockStacker->Row2Block1SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row2Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row2Block1SpriteId].x = (gSprites[sBlockStacker->Row2Block1SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 3) && (sBlockStacker->BlocksLeft == 2)) // Level 3, 2 Lives
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row3Block2SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row3Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row3Block2SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row3Block1SpriteId].x = (gSprites[sBlockStacker->Row3Block1SpriteId].x + 16);
				gSprites[sBlockStacker->Row3Block2SpriteId].x = (gSprites[sBlockStacker->Row3Block2SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row3Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row3Block1SpriteId].x = (gSprites[sBlockStacker->Row3Block1SpriteId].x - 16);
				gSprites[sBlockStacker->Row3Block2SpriteId].x = (gSprites[sBlockStacker->Row3Block2SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 3) && (sBlockStacker->BlocksLeft == 1)) // Level 3, 1 Lives
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row3Block1SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row3Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row3Block1SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row3Block1SpriteId].x = (gSprites[sBlockStacker->Row3Block1SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row3Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row3Block1SpriteId].x = (gSprites[sBlockStacker->Row3Block1SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 4) && (sBlockStacker->BlocksLeft == 2)) // Level 4, 2 Lives
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row4Block2SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row4Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row4Block2SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row4Block1SpriteId].x = (gSprites[sBlockStacker->Row4Block1SpriteId].x + 16);
				gSprites[sBlockStacker->Row4Block2SpriteId].x = (gSprites[sBlockStacker->Row4Block2SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row4Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row4Block1SpriteId].x = (gSprites[sBlockStacker->Row4Block1SpriteId].x - 16);
				gSprites[sBlockStacker->Row4Block2SpriteId].x = (gSprites[sBlockStacker->Row4Block2SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 4) && (sBlockStacker->BlocksLeft == 1)) // Level 4, 1 Live
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row4Block1SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row4Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row4Block1SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row4Block1SpriteId].x = (gSprites[sBlockStacker->Row4Block1SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row4Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row4Block1SpriteId].x = (gSprites[sBlockStacker->Row4Block1SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 5) && (sBlockStacker->BlocksLeft == 1)) // Level 5, 1 Live
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row5Block1SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row5Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row5Block1SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row5Block1SpriteId].x = (gSprites[sBlockStacker->Row5Block1SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row5Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row5Block1SpriteId].x = (gSprites[sBlockStacker->Row5Block1SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 6) && (sBlockStacker->BlocksLeft == 1)) // Level 6, 1 Live
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row6Block1SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row6Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row6Block1SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row6Block1SpriteId].x = (gSprites[sBlockStacker->Row6Block1SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row6Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row6Block1SpriteId].x = (gSprites[sBlockStacker->Row6Block1SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 7) && (sBlockStacker->BlocksLeft == 1)) // Level 7, 1 Live
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row7Block1SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row7Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row7Block1SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row7Block1SpriteId].x = (gSprites[sBlockStacker->Row7Block1SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row7Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row7Block1SpriteId].x = (gSprites[sBlockStacker->Row7Block1SpriteId].x - 16);
			}
		}
		else if ((sBlockStacker->CurrentRow == 8) && (sBlockStacker->BlocksLeft == 1)) // Level 8, 1 Live
		{
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row8Block1SpriteId].x == 128)) // Right, against right side
			{
				sBlockStacker->xDirection = 1;
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row8Block1SpriteId].x == 32)) // Left, against left side
			{
				sBlockStacker->xDirection = 0;
			}
			
			if ((sBlockStacker->xDirection == 0) && (gSprites[sBlockStacker->Row8Block1SpriteId].x != 128)) // Right, not against right side
			{
				gSprites[sBlockStacker->Row8Block1SpriteId].x = (gSprites[sBlockStacker->Row8Block1SpriteId].x + 16);
			}
			else if ((sBlockStacker->xDirection == 1) && (gSprites[sBlockStacker->Row8Block1SpriteId].x != 32)) // Left, not against left side
			{
				gSprites[sBlockStacker->Row8Block1SpriteId].x = (gSprites[sBlockStacker->Row8Block1SpriteId].x - 16);
			}
		}
	}
	sBlockStacker->xSpeedDelay--;
}

static void AButton(void)
{
	PlaySE(SE_M_STRENGTH);
	//SwapFromBlock();
	sBlockStacker->ToggleButtons = 0;
	
	if ((sBlockStacker->CurrentRow == 1) && (sBlockStacker->BlocksLeft == 3)) // Level 1, 3 Lives
	{
		gSprites[sBlockStacker->Row1Block1SpriteId].oam.tileNum += 4;
		gSprites[sBlockStacker->Row1Block2SpriteId].oam.tileNum += 4;
		gSprites[sBlockStacker->Row1Block3SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 2) && (sBlockStacker->BlocksLeft == 3)) // Level 2, 3 Lives
	{
		gSprites[sBlockStacker->Row2Block1SpriteId].oam.tileNum += 4;
		gSprites[sBlockStacker->Row2Block2SpriteId].oam.tileNum += 4;
		gSprites[sBlockStacker->Row2Block3SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 2) && (sBlockStacker->BlocksLeft == 2)) // Level 2, 2 Lives
	{
		gSprites[sBlockStacker->Row2Block1SpriteId].oam.tileNum += 4;
		gSprites[sBlockStacker->Row2Block2SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 2) && (sBlockStacker->BlocksLeft == 1)) // Level 2, 1 Live
	{
		gSprites[sBlockStacker->Row2Block1SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 3) && (sBlockStacker->BlocksLeft == 2)) // Level 3, 2 Lives
	{
		gSprites[sBlockStacker->Row3Block1SpriteId].oam.tileNum += 4;
		gSprites[sBlockStacker->Row3Block2SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 3) && (sBlockStacker->BlocksLeft == 1)) // Level 3, 1 Live
	{
		gSprites[sBlockStacker->Row3Block1SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 4) && (sBlockStacker->BlocksLeft == 2)) // Level 4, 2 Lives
	{
		gSprites[sBlockStacker->Row4Block1SpriteId].oam.tileNum += 4;
		gSprites[sBlockStacker->Row4Block2SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 4) && (sBlockStacker->BlocksLeft == 1)) // Level 4, 1 Live
	{
		gSprites[sBlockStacker->Row4Block1SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 5) && (sBlockStacker->BlocksLeft == 1)) // Level 5, 1 Live
	{
		gSprites[sBlockStacker->Row5Block1SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 6) && (sBlockStacker->BlocksLeft == 1)) // Level 6, 1 Live
	{
		gSprites[sBlockStacker->Row6Block1SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 7) && (sBlockStacker->BlocksLeft == 1)) // Level 7, 1 Live
	{
		gSprites[sBlockStacker->Row7Block1SpriteId].oam.tileNum += 4;
	}
	else if ((sBlockStacker->CurrentRow == 8) && (sBlockStacker->BlocksLeft == 1)) // Level 8, 1 Live
	{
		gSprites[sBlockStacker->Row8Block1SpriteId].oam.tileNum += 4;
	}
	sBlockStacker->state = STACKER_CHECK_POS;
}

static void CheckLevel_2(void)
{
	s16 curX1;
	s16 curX2;
	s16 curX3;
	s16 preX1;
	s16 preX2;
	s16 preX3;
	u8 Lives;
	
	curX1 = gSprites[sBlockStacker->Row2Block1SpriteId].x;
	curX2 = gSprites[sBlockStacker->Row2Block2SpriteId].x;
	curX3 = gSprites[sBlockStacker->Row2Block3SpriteId].x;
	
	preX1 = gSprites[sBlockStacker->Row1Block1SpriteId].x;
	preX2 = gSprites[sBlockStacker->Row1Block2SpriteId].x;
	preX3 = gSprites[sBlockStacker->Row1Block3SpriteId].x;
	
	Lives = sBlockStacker->BlocksLeft;
	sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
	
	if ((curX1 != preX1) && (curX1 != preX2) && (curX1 != preX3)) // Block 1 Off
	{
		sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
		sBlockStacker->BlocksLeft--;
		sBlockStacker->x1Active = 1;
		CreateX1(gSprites[sBlockStacker->Row2Block1SpriteId].x, gSprites[sBlockStacker->Row2Block1SpriteId].y);
		gSprites[sBlockStacker->Row2Block1SpriteId].invisible = TRUE;
		gSprites[sBlockStacker->Row2Block1SpriteId].x = 200;
		//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row2Block1SpriteId]);
	}
	if (Lives > 1)
	{
		if ((curX2 != preX1) && (curX2 != preX2) && (curX2 != preX3)) // Block 2 Off
		{
			sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
			sBlockStacker->BlocksLeft--;
			sBlockStacker->x2Active = 1;
			CreateX2(gSprites[sBlockStacker->Row2Block2SpriteId].x, gSprites[sBlockStacker->Row2Block2SpriteId].y);
			gSprites[sBlockStacker->Row2Block2SpriteId].invisible = TRUE;
			gSprites[sBlockStacker->Row2Block2SpriteId].x = 200;
			//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row2Block2SpriteId]);
		}
	}
	if (Lives > 2)
	{
		if ((curX3 != preX1) && (curX3 != preX2) && (curX3 != preX3)) // Block 3 Off
		{
			sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
			sBlockStacker->BlocksLeft--;
			sBlockStacker->x3Active = 1;
			CreateX3(gSprites[sBlockStacker->Row2Block3SpriteId].x, gSprites[sBlockStacker->Row2Block3SpriteId].y);
			gSprites[sBlockStacker->Row2Block3SpriteId].invisible = TRUE;
			gSprites[sBlockStacker->Row2Block3SpriteId].x = 200;
			//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row2Block3SpriteId]);
		}
	}
}

static void CheckLevel_3(void)
{
	s16 curX1;
	s16 curX2;
	s16 curX3;
	s16 preX1;
	s16 preX2;
	s16 preX3;
	u8 Lives;
	
	curX1 = gSprites[sBlockStacker->Row3Block1SpriteId].x;
	curX2 = gSprites[sBlockStacker->Row3Block2SpriteId].x;
	
	preX1 = gSprites[sBlockStacker->Row2Block1SpriteId].x;
	preX2 = gSprites[sBlockStacker->Row2Block2SpriteId].x;
	preX3 = gSprites[sBlockStacker->Row2Block3SpriteId].x;
	
	Lives = sBlockStacker->BlocksLeft;
	sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
	
	if ((curX1 != preX1) && (curX1 != preX2) && (curX1 != preX3)) // Block 1 Off
	{
		sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
		sBlockStacker->BlocksLeft--;
		sBlockStacker->x1Active = 1;
		CreateX1(gSprites[sBlockStacker->Row3Block1SpriteId].x, gSprites[sBlockStacker->Row3Block1SpriteId].y);
		gSprites[sBlockStacker->Row3Block1SpriteId].invisible = TRUE;
		gSprites[sBlockStacker->Row3Block1SpriteId].x = 200;
		//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row3Block1SpriteId]);
	}
	if (Lives > 1)
	{
		if ((curX2 != preX1) && (curX2 != preX2) && (curX2 != preX3)) // Block 2 Off
		{
			sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
			sBlockStacker->BlocksLeft--;
			sBlockStacker->x2Active = 1;
			CreateX2(gSprites[sBlockStacker->Row3Block2SpriteId].x, gSprites[sBlockStacker->Row3Block2SpriteId].y);
			gSprites[sBlockStacker->Row3Block2SpriteId].invisible = TRUE;
			gSprites[sBlockStacker->Row3Block2SpriteId].x = 200;
			//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row3Block2SpriteId]);
		}
	}
}

static void CheckLevel_4(void)
{
	s16 curX1;
	s16 curX2;
	s16 curX3;
	s16 preX1;
	s16 preX2;
	s16 preX3;
	u8 Lives;
	
	curX1 = gSprites[sBlockStacker->Row4Block1SpriteId].x;
	curX2 = gSprites[sBlockStacker->Row4Block2SpriteId].x;
	
	preX1 = gSprites[sBlockStacker->Row3Block1SpriteId].x;
	preX2 = gSprites[sBlockStacker->Row3Block2SpriteId].x;
	
	Lives = sBlockStacker->BlocksLeft;
	sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
	
	if ((curX1 != preX1) && (curX1 != preX2)) // Block 1 Off
	{
		sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
		sBlockStacker->BlocksLeft--;
		sBlockStacker->x1Active = 1;
		CreateX1(gSprites[sBlockStacker->Row4Block1SpriteId].x, gSprites[sBlockStacker->Row4Block1SpriteId].y);
		gSprites[sBlockStacker->Row4Block1SpriteId].invisible = TRUE;
		gSprites[sBlockStacker->Row4Block1SpriteId].x = 200;
		//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row4Block1SpriteId]);
	}
	if (Lives > 1)
	{
		if ((curX2 != preX1) && (curX2 != preX2)) // Block 2 Off
		{
			sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
			sBlockStacker->BlocksLeft--;
			sBlockStacker->x2Active = 1;
			CreateX2(gSprites[sBlockStacker->Row4Block2SpriteId].x, gSprites[sBlockStacker->Row4Block2SpriteId].y);
			gSprites[sBlockStacker->Row4Block2SpriteId].invisible = TRUE;
			gSprites[sBlockStacker->Row4Block2SpriteId].x = 200;
			//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row4Block2SpriteId]);
		}
	}
}

static void CheckLevel_5(void)
{
	s16 curX1;
	s16 curX2;
	s16 curX3;
	s16 preX1;
	s16 preX2;
	s16 preX3;
	u8 Lives;
	
	curX1 = gSprites[sBlockStacker->Row5Block1SpriteId].x;
	
	preX1 = gSprites[sBlockStacker->Row4Block1SpriteId].x;
	preX2 = gSprites[sBlockStacker->Row4Block2SpriteId].x;
	
	Lives = sBlockStacker->BlocksLeft;
	sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
	
	if ((curX1 != preX1) && (curX1 != preX2)) // Block 1 Off
	{
		sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
		sBlockStacker->BlocksLeft--;
		sBlockStacker->x1Active = 1;
		CreateX1(gSprites[sBlockStacker->Row5Block1SpriteId].x, gSprites[sBlockStacker->Row5Block1SpriteId].y);
		gSprites[sBlockStacker->Row5Block1SpriteId].invisible = TRUE;
		gSprites[sBlockStacker->Row5Block1SpriteId].x = 200;
		//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row5Block1SpriteId]);
	}
}

static void CheckLevel_6(void)
{
	s16 curX1;
	s16 curX2;
	s16 curX3;
	s16 preX1;
	s16 preX2;
	s16 preX3;
	u8 Lives;
	
	curX1 = gSprites[sBlockStacker->Row6Block1SpriteId].x;
	
	preX1 = gSprites[sBlockStacker->Row5Block1SpriteId].x;
	
	Lives = sBlockStacker->BlocksLeft;
	sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
	
	if ((curX1 != preX1)) // Block 1 Off
	{
		sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
		sBlockStacker->BlocksLeft--;
		sBlockStacker->x1Active = 1;
		CreateX1(gSprites[sBlockStacker->Row6Block1SpriteId].x, gSprites[sBlockStacker->Row6Block1SpriteId].y);
		gSprites[sBlockStacker->Row6Block1SpriteId].invisible = TRUE;
		gSprites[sBlockStacker->Row6Block1SpriteId].x = 200;
		//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row6Block1SpriteId]);
	}
}

static void CheckLevel_7(void)
{
	s16 curX1;
	s16 curX2;
	s16 curX3;
	s16 preX1;
	s16 preX2;
	s16 preX3;
	u8 Lives;
	
	curX1 = gSprites[sBlockStacker->Row7Block1SpriteId].x;
	
	preX1 = gSprites[sBlockStacker->Row6Block1SpriteId].x;
	
	Lives = sBlockStacker->BlocksLeft;
	sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
	
	if ((curX1 != preX1)) // Block 1 Off
	{
		sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
		sBlockStacker->BlocksLeft--;
		sBlockStacker->x1Active = 1;
		CreateX1(gSprites[sBlockStacker->Row7Block1SpriteId].x, gSprites[sBlockStacker->Row7Block1SpriteId].y);
		gSprites[sBlockStacker->Row7Block1SpriteId].invisible = TRUE;
		gSprites[sBlockStacker->Row7Block1SpriteId].x = 200;
		//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row7Block1SpriteId]);
	}
}

static void CheckLevel_8(void)
{
	s16 curX1;
	s16 curX2;
	s16 curX3;
	s16 preX1;
	s16 preX2;
	s16 preX3;
	u8 Lives;
	
	curX1 = gSprites[sBlockStacker->Row8Block1SpriteId].x;
	
	preX1 = gSprites[sBlockStacker->Row7Block1SpriteId].x;
	
	Lives = sBlockStacker->BlocksLeft;
	sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
	
	if ((curX1 != preX1)) // Block 1 Off
	{
		sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
		sBlockStacker->BlocksLeft--;
		sBlockStacker->x1Active = 1;
		CreateX1(gSprites[sBlockStacker->Row8Block1SpriteId].x, gSprites[sBlockStacker->Row8Block1SpriteId].y);
		gSprites[sBlockStacker->Row8Block1SpriteId].invisible = TRUE;
		gSprites[sBlockStacker->Row8Block1SpriteId].x = 200;
		//DestroySpriteAndFreeResources(&gSprites[sBlockStacker->Row8Block1SpriteId]);
	}
}

static void HandleInput(void)
{
	if (sBlockStacker->ToggleButtons == 1) 
	{
		if (JOY_NEW(A_BUTTON))
		{
			AButton();
		}
		else if (JOY_NEW(B_BUTTON))
		{
			if (sBlockStacker->exitToggle == 0) {
			PlaySE(SE_SELECT);
			sBlockStacker->state = STACKER_START_EXIT;
			}
		}
	}
}

static void HandleInput2(void)
{
	if (JOY_NEW(A_BUTTON))
	{
		if (sBlockStacker->YesNo == 0) // Yes
		{
			PlaySE(SE_POKENAV_ON);
			gSprites[sBlockStacker->YesSpriteId].invisible = TRUE;
			gSprites[sBlockStacker->NoSpriteId].invisible = TRUE;
			DestroyKeepGoing();
			DestroySpriteAndFreeResources(&gSprites[sBlockStacker->YesSpriteId]);
			DestroySpriteAndFreeResources(&gSprites[sBlockStacker->NoSpriteId]);
			sBlockStacker->GoDelay = 30;
			sBlockStacker->state = STACKER_POST_DELAY;
			return;
		}
		else
		{
			PlaySE(SE_POKENAV_OFF);
			sBlockStacker->YesNo = 1;
			gSprites[sBlockStacker->YesSpriteId].invisible = TRUE;
			gSprites[sBlockStacker->NoSpriteId].invisible = TRUE;
			DestroyKeepGoing();
			DestroySpriteAndFreeResources(&gSprites[sBlockStacker->YesSpriteId]);
			DestroySpriteAndFreeResources(&gSprites[sBlockStacker->NoSpriteId]);
			sBlockStacker->GoDelay = 30;
			sBlockStacker->state = STACKER_POST_DELAY;
			return;
		}			
	}
	else if (JOY_NEW(B_BUTTON))
	{
			PlaySE(SE_POKENAV_OFF);
			gSprites[sBlockStacker->YesSpriteId].invisible = TRUE;
			gSprites[sBlockStacker->NoSpriteId].invisible = TRUE;
			DestroyKeepGoing();
			DestroySpriteAndFreeResources(&gSprites[sBlockStacker->YesSpriteId]);
			DestroySpriteAndFreeResources(&gSprites[sBlockStacker->NoSpriteId]);
			sBlockStacker->GoDelay = 30;
			sBlockStacker->state = STACKER_POST_DELAY;
			return;
	}
	else if (JOY_NEW(DPAD_RIGHT))
	{
		if (sBlockStacker->YesNo == 0)
		{
			PlaySE(SE_SELECT);
			sBlockStacker->YesNo = 1; // No
			gSprites[sBlockStacker->YesSpriteId].oam.tileNum += 32;
			gSprites[sBlockStacker->NoSpriteId].oam.tileNum -= 32;
		}
		return;
	}
	else if (JOY_NEW(DPAD_LEFT))
	{
		if (sBlockStacker->YesNo == 1)
		{
			PlaySE(SE_SELECT);
			sBlockStacker->YesNo = 0; // Yes
			gSprites[sBlockStacker->YesSpriteId].oam.tileNum -= 32;
			gSprites[sBlockStacker->NoSpriteId].oam.tileNum += 32;
		}
		return;
	}
}

static void ExitBlockStacker(void)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        FREE_AND_SET_NULL(sBlockStacker);
    }
}

static void StartExitBlockStacker(void)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    sBlockStacker->state = STACKER_EXIT;
}

static void BlockStackerMain(u8 taskId)
{
	switch (sBlockStacker->state)
	{
		case STACKER_HIGHLIGHT:
			CreateHighlight(sBlockStacker->HighlightNum, sBlockStacker->HighlightRow);
			DestroyHighlights();
			if (sBlockStacker->DestroyedHighlights > 55)
			{
				sBlockStacker->state = STACKER_HIGHLIGHT_END;
			}
			break;	
		case STACKER_HIGHLIGHT_END:
			sBlockStacker->GoDelay = 50;
			sBlockStacker->state = STACKER_GO_DELAY;
			break;		
		case STACKER_GO_DELAY:
			sBlockStacker->GoDelay--;
			if (sBlockStacker->GoDelay == 0)
			{
				DestroyTitle();
				sBlockStacker->GoDelay = 100;
				sBlockStacker->state = STACKER_GO;
			}
			break;
		case STACKER_GO:
			if (sBlockStacker->GoDelay == 100)
			{
				PlaySE(SE_POKENAV_ON);
				CreateStart();
			}
			sBlockStacker->GoDelay--;
			if (sBlockStacker->GoDelay == 0)
			{
				PlaySE(SE_EGG_HATCH);
				DestroyStart();
				CreateCommands();
				CreateArrow();
				CreateLives();
				//SwapToBlock();
				sBlockStacker->state = STACKER_LEVEL_SETUP;
			}
			break;
		case STACKER_LEVEL_SETUP:
			gSprites[sBlockStacker->ArrowSpriteId].y = 153 - (16 * sBlockStacker->CurrentRow);
			
			if (sBlockStacker->CurrentRow == 1)
			{
				CreateLevel_1();
			}
			else if (sBlockStacker->CurrentRow == 2)
			{
				CreateLevel_2();
			}
			else if (sBlockStacker->CurrentRow == 3)
			{
				if (sBlockStacker->BlocksLeft > 2)
				{
					sBlockStacker->BlocksLeft = 2;
				}
				CreateLevel_3();
			}
			else if (sBlockStacker->CurrentRow == 4)
			{
				CreateLevel_4();
			}
			else if (sBlockStacker->CurrentRow == 5)
			{
				if (sBlockStacker->BlocksLeft > 1)
				{
					sBlockStacker->BlocksLeft = 1;
				}
				CreateLevel_5();
			}
			else if (sBlockStacker->CurrentRow == 6)
			{
				CreateLevel_6();
			}
			else if (sBlockStacker->CurrentRow == 7)
			{
				CreateLevel_7();
				PlayBGM(MUS_RG_TRAINER_TOWER);
			}
			else if (sBlockStacker->CurrentRow == 8)
			{
				CreateLevel_8();
				PlayBGM(MUS_RG_SEVII_ROUTE);
			}
			UpdateLives();
			sBlockStacker->ToggleButtons = 1;
			sBlockStacker->state = STACKER_INPUT;
			break;	
		case STACKER_INPUT:
			UpdateBlockPosition();
			HandleInput();
			break;
		case STACKER_CHECK_POS:
			if (sBlockStacker->CurrentRow == 2)
			{
				CheckLevel_2();
			}
			else if (sBlockStacker->CurrentRow == 3)
			{
				CheckLevel_3();
			}
			else if (sBlockStacker->CurrentRow == 4)
			{
				CheckLevel_4();
			}
			else if (sBlockStacker->CurrentRow == 5)
			{
				CheckLevel_5();
			}
			else if (sBlockStacker->CurrentRow == 6)
			{
				CheckLevel_6();
			}
			else if (sBlockStacker->CurrentRow == 7)
			{
				CheckLevel_7();
			}
			else if (sBlockStacker->CurrentRow == 8)
			{
				CheckLevel_8();
			}
			
			sBlockStacker->GoDelay = 30;
			sBlockStacker->state = STACKER_ROW_DELAY;
			break;		
		case STACKER_ROW_DELAY:
			sBlockStacker->GoDelay--;
			if (sBlockStacker->GoDelay == 15 && ((sBlockStacker->x1Active != 0) || (sBlockStacker->x2Active != 0) || (sBlockStacker->x3Active != 0)))
			{
				PlaySE(SE_FAILURE);
			}
			if (sBlockStacker->GoDelay == 0)
			{
				if (sBlockStacker->x1Active != 0)
				{
					DestroySpriteAndFreeResources(&gSprites[sBlockStacker->x1SpriteId]);
					sBlockStacker->x1Active = 0;
				}
				if (sBlockStacker->x2Active != 0)
				{
					DestroySpriteAndFreeResources(&gSprites[sBlockStacker->x2SpriteId]);
					sBlockStacker->x2Active = 0;
				}
				if (sBlockStacker->x3Active != 0)
				{
					DestroySpriteAndFreeResources(&gSprites[sBlockStacker->x3SpriteId]);
					sBlockStacker->x3Active = 0;
				}
				if ((sBlockStacker->BlocksLeft > 0) && (sBlockStacker->CurrentRow != 6) && (sBlockStacker->CurrentRow != 7) && (sBlockStacker->CurrentRow != 8))
				{
					//sBlockStacker->ToggleButtons = 1;
					sBlockStacker->CurrentRow++;
					PlaySE(SE_EGG_HATCH);
					//SwapToBlock();
					sBlockStacker->state = STACKER_LEVEL_SETUP;
				}
				else if (sBlockStacker->BlocksLeft == 0)
				{
					sBlockStacker->GoDelay = 140;
					PlayFanfare(MUS_TOO_BAD);
					CreateGameOver();
					DestroyLives();
					sBlockStacker->state = STACKER_GAME_OVER;
				}
				else if ((sBlockStacker->BlocksLeft > 0) && (sBlockStacker->CurrentRow == 6))
				{
					sBlockStacker->Winnings = 50;
					sBlockStacker->CurrentRow++;
					CreateKeepGoing();
					CreateYesNo();
					PlayFanfare(MUS_LEVEL_UP);
					sBlockStacker->state = STACKER_KEEP_GOING;
				}
				else if ((sBlockStacker->BlocksLeft > 0) && (sBlockStacker->CurrentRow == 7))
				{
					sBlockStacker->Winnings = 250;
					sBlockStacker->CurrentRow++;
					CreateKeepGoing();
					CreateYesNo();
					PlayFanfare(MUS_LEVEL_UP);
					sBlockStacker->state = STACKER_KEEP_GOING;
				}
				else if ((sBlockStacker->BlocksLeft > 0) && (sBlockStacker->CurrentRow == 8))
				{
					sBlockStacker->Winnings = 1000;
					sBlockStacker->CurrentRow++;
					PlayFanfare(MUS_OBTAIN_BADGE);
					sBlockStacker->GoDelay = 330;
					CreateWinner();
					sBlockStacker->state = STACKER_WIN;
				}
			}
			break;			
		case STACKER_KEEP_GOING:
			HandleInput2();
			break;
		case STACKER_POST_DELAY:
			sBlockStacker->GoDelay--;
			if (sBlockStacker->GoDelay == 0)
			{
				if (sBlockStacker->YesNo == 1) // No
				{
					VarSet(VAR_FLIP_WINNINGS, sBlockStacker->Winnings);
					sBlockStacker->state = STACKER_START_EXIT;
				}
				else
				{
					PlaySE(SE_EGG_HATCH);
					//SwapToBlock();
					sBlockStacker->state = STACKER_LEVEL_SETUP;
				}
			}
			break;	
		case STACKER_WIN:
			sBlockStacker->GoDelay--;
			if (sBlockStacker->GoDelay == 0)
			{
				VarSet(VAR_FLIP_WINNINGS, sBlockStacker->Winnings);
				sBlockStacker->state = STACKER_START_EXIT;
			}
			break;
		case STACKER_GAME_OVER:
			sBlockStacker->GoDelay--;
			if (sBlockStacker->GoDelay == 0)
			{
				sBlockStacker->state = STACKER_START_EXIT;
			}
			break;
		case STACKER_START_EXIT:
			StartExitBlockStacker();
			break;
		case STACKER_EXIT:
			ExitBlockStacker();
			break;	
	}
}

static void InitBlockStackerScreen(void)
{	
    SetVBlankCallback(NULL);
    ResetAllBgsCoordinates();
    ResetVramOamAndBgCntRegs();
    ResetBgsAndClearDma3BusyFlags(0);
    ResetTempTileDataBuffers();
	InitBgsFromTemplates(0, sBlockStackerBGtemplates, ARRAY_COUNT(sBlockStackerBGtemplates));
	SetBgTilemapBuffer(BLOCKSTACKER_BG, AllocZeroed(BG_SCREEN_SIZE));
	DecompressAndLoadBgGfxUsingHeap(BLOCKSTACKER_BG, BlockStacker_BG_Img, 0x1140, 0, 0);
	CopyToBgTilemapBuffer(BLOCKSTACKER_BG, BlockStacker_Tilemap, 0, 0);
	ResetPaletteFade();
	LoadPalette(BlockStacker_BG_Pal, 0, sizeof(BlockStacker_BG_Pal));
	ResetSpriteData();
    FreeAllSpritePalettes();
	LoadSpritePalettes(sSpritePalettes);
	
	CreateRhydon();
	CreateTitle();
	sBlockStacker->HighlightNum = 0; // 0-13
	sBlockStacker->HighlightRow = 0; // 0-7
	sBlockStacker->DestroyedHighlights = 0;
	sBlockStacker->ToggleButtons = 0;
	sBlockStacker->CurrentRow = 1;
	sBlockStacker->BlocksLeft = 3;
	sBlockStacker->LastLives = sBlockStacker->BlocksLeft;
	
	CopyBgTilemapBufferToVram(BLOCKSTACKER_BG);
	//CopyBgTilemapBufferToVram(BLOCKSTACKER_TEXT_MENUS);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_BG2_ON);
    ShowBg(BLOCKSTACKER_BG);
	//ShowBg(BLOCKSTACKER_TEXT_MENUS);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(BlockStackerVBlankCallback);
    SetMainCallback2(BlockStackerMainCallback);
	PlaySE(SE_MUGSHOT);
    CreateTask(BlockStackerMain, 1);
}
