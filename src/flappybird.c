#include "flappybird.h"
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
#include "trig.h"
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
	FLAPPY_INIT,

	FLAPPY_INPUT,
	FLAPPY_STOP,
	FLAPPY_ROTATION,
	FLAPPY_GAMEOVER,
	FLAPPY_HISCORE,
	FLAPPY_START_EXIT,
	FLAPPY_EXIT,
};

enum {
    SPR_HISCORE_DIG_1,
    SPR_HISCORE_DIG_10,
    SPR_HISCORE_DIG_100,
    SPR_HISCORE_DIG_1000,
};

enum {
    SPR_CREDIT_DIG_1,
    SPR_CREDIT_DIG_10,
    SPR_CREDIT_DIG_100,
    SPR_CREDIT_DIG_1000,
};

#define SPR_CREDIT_DIGITS SPR_CREDIT_DIG_1
#define SPR_HISCORE_DIGITS SPR_HISCORE_DIG_1

#define MAX_SPRITES_CREDIT 4
#define MAX_SPRITES_HISCORE 4

struct FlappyBird {
	u8 state;
	u8 CreditSpriteIds[MAX_SPRITES_CREDIT];
	u8 ButterfreeSpriteId;
	u8 ButterfreeHitboxSpriteId;
	u16 scroll_fg_x_int;
	u16 scroll_bg_x_int;
	u16 scroll_fg_x_frac;
	u16 scroll_bg_x_frac;
	u16 pos_y;           // Bird's vertical position
    s16 speed_y;         // Bird's vertical speed (falling speed)
    u8 flap_strength;    // Strength of the flap (negative speed change when flap occurs)
	u32 delay;
	u16 bg2ScrollX; //0 -256
	u8 Obstacle1Id;
	u8 Obstacle2Id;
	u16 MAX_Y;
	u16 MIN_Y;
	u8 TrailSpriteId;
	s16 TrailoffsetX, TrailoffsetY;
	u16 timerDelay;
	u8 isFlapping;
	u8 SFX;
	u8 timer;
	u8 BorderSpriteId;
	u8 DamageSpriteId;
	u8 RotateToggle;
	s16 DamageSpriteRotation;
	u8 yUpdate;
	s16 DamageSpriteY;
	u8 jumping;
	s16 jumpHeight;
	s16 jumpSpeed;
	s16 fallSpeed;
	s16 originalY;
	s16 velocity;
	u8 BorderSprite2Id;
	u8 StartSpriteId;
	u8 GameOverSpriteId;
	u8 OneSpriteId;
	u8 TwoSpriteId;
	u8 ThreeSpriteId;
	u8 HiScoreSpriteId;
	u32 Points;
	u8 FlapSpriteId;
	u8 ScoreSpriteId;
	u8 Score2SpriteId;
	u8 ScoreSpriteIds[MAX_SPRITES_HISCORE];
};	

static EWRAM_DATA struct FlappyBird *sFlappy = NULL;
static EWRAM_DATA u8 sTextWindowId = 0;

#define MAX_Y_POSITION 10  // The highest point the bird can reach (lower is higher on the screen)
#define MIN_Y_POSITION 140 // The ground level (Y position of the ground)

#define GRAVITY 1    // Slower gravity (lower value = slower fall)
#define FLAP_STRENGTH -6  // Negative value to make the bird go up

static void FadeToFlappyBirdScreen(u8 taskId);
static void InitFlappyBirdScreen(void);
static void FlappyBirdVBlankCallback(void);

// Backgound

static const u32 FlappyBird_BG_Img[] = INCBIN_U32("graphics/flappybird/flappy-bg.4bpp.lz");
static const u8 FlappyBird_BG_Tilemap[] = INCBIN_U8("graphics/flappybird/flappy-bg.bin.lz");
static const u16 FlappyBird_BG_Pal[] = INCBIN_U16("graphics/flappybird/flappy-bg.gbapal");

// Foreground

static const u32 FlappyBird_FG_Img[] = INCBIN_U32("graphics/flappybird/flappy-fg.4bpp.lz");
static const u8 FlappyBird_FG_Tilemap[] = INCBIN_U8("graphics/flappybird/flappy-fg.bin.lz");
static const u16 FlappyBird_FG_Pal[] = INCBIN_U16("graphics/flappybird/flappy-fg.gbapal");

// Arcade BG Element

static const u32 Arcade_FG_Img[] = INCBIN_U32("graphics/flappybird/arcade-screen.4bpp.lz");
static const u8 Arcade_FG_Tilemap[] = INCBIN_U8("graphics/flappybird/arcade-screen.bin.lz");
static const u16 Arcade_FG_Pal[] = INCBIN_U16("graphics/flappybird/arcade-screen.gbapal");

// Butterfree
static const u32 ButterfreeGFX[] = INCBIN_U32("graphics/flappybird/butterfree.4bpp.lz");
static const u16 ButterfreePAL[] = INCBIN_U16("graphics/flappybird/butterfree.gbapal");

// Hitbox
static const u32 ButterfreeHitboxGFX[] = INCBIN_U32("graphics/flappybird/butterfree-hitbox.4bpp.lz");
static const u16 ButterfreeHitboxPAL[] = INCBIN_U16("graphics/flappybird/butterfree-hitbox.gbapal");

// Trail
static const u32 TrailGFX[] = INCBIN_U32("graphics/flappybird/trail.4bpp.lz");
static const u16 TrailPAL[] = INCBIN_U16("graphics/flappybird/trail.gbapal");

// Cover

static const u32 TopLeftGFX[] = INCBIN_U32("graphics/flappybird/topleft.4bpp.lz");
static const u32 BottomLeftGFX[] = INCBIN_U32("graphics/flappybird/bottomleft.4bpp.lz");

// Damage

static const u32 DamageGFX[] = INCBIN_U32("graphics/flappybird/damage.4bpp.lz");

// Text

static const u32 StartGFX[] = INCBIN_U32("graphics/flappybird/start.4bpp.lz");
static const u32 GameOverGFX[] = INCBIN_U32("graphics/flappybird/gameover.4bpp.lz");
static const u32 HiScoreGFX[] = INCBIN_U32("graphics/flappybird/hiscore.4bpp.lz");
static const u32 OneGFX[] = INCBIN_U32("graphics/flappybird/one.4bpp.lz");
static const u32 TwoGFX[] = INCBIN_U32("graphics/flappybird/two.4bpp.lz");
static const u32 ThreeGFX[] = INCBIN_U32("graphics/flappybird/three.4bpp.lz");
static const u32 FlapGFX[] = INCBIN_U32("graphics/flappybird/flap.4bpp.lz");
static const u32 ScoreGFX[] = INCBIN_U32("graphics/flappybird/score.4bpp.lz");
static const u32 Score2GFX[] = INCBIN_U32("graphics/flappybird/score2.4bpp.lz");
static const u32 DigitsGFX[] = INCBIN_U32("graphics/flappybird/digits.4bpp.lz");

static const u16 TextPAL[] = INCBIN_U16("graphics/flappybird/text.gbapal");
static const u16 FlapPAL[] = INCBIN_U16("graphics/flappybird/flap.gbapal");

#define FlappyBird_BG 3
#define FlappyBird_FG 2
#define Arcade_BG 1
#define FlappyBird_TEXT_MENUS 0

static const struct BgTemplate sFlappyBGtemplates[] = {
    {
       .bg = FlappyBird_BG,
       .charBaseIndex = 3,
       .mapBaseIndex = 29,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 3,
       .baseTile = 0
   },
   {
       .bg = FlappyBird_FG,
       .charBaseIndex = 2,
       .mapBaseIndex = 28,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 2,
       .baseTile = 0
   },
   {
       .bg = Arcade_BG,
       .charBaseIndex = 1,
       .mapBaseIndex = 27,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 1,
       .baseTile = 0
   },
   {
        .bg = FlappyBird_TEXT_MENUS,
        .charBaseIndex = 0,
        .mapBaseIndex = 0x17,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

static const struct WindowTemplate sFlappyWinTemplates[] = {
	{
        .bg = FlappyBird_TEXT_MENUS,
        .tilemapLeft = 16,
        .tilemapTop = 9,
        .width = 14,
        .height = 2,
        .paletteNum = 0xF,
        .baseBlock = 0x194,		
	},
    DUMMY_WIN_TEMPLATE,
};

#define BUTTERFREE_GFXTAG 1
#define BUTTERFREE_HITBOX_GFXTAG 2
#define TRAIL_GFXTAG 3
#define BORDER_GFXTAG 4
#define DAMAGE_GFXTAG 5
#define BORDER_2_GFXTAG 6
#define START_GFXTAG 7
#define GAMEOVER_GFXTAG 8
#define THREE_GFXTAG 9
#define TWO_GFXTAG 10
#define ONE_GFXTAG 11
#define HISCORE_GFXTAG 12
#define FLAP_GFXTAG 13
#define SCORE_GFXTAG 14
#define SCORE2_GFXTAG 15
#define CREDIT_DIGITS_GFXTAG 16

#define BUTTERFREE_PALTAG 1
#define BUTTERFREE_HITBOX_PALTAG 2
#define TRAIL_PALTAG 3
#define BORDER_PALTAG 4
#define TEXT_PALTAG 5
#define FLAP_PALTAG 6

static const struct SpritePalette sSpritePalettes[] =
{
    { .data = ButterfreePAL,      	.tag = BUTTERFREE_PALTAG },
	{ .data = ButterfreeHitboxPAL,	.tag = BUTTERFREE_HITBOX_PALTAG },
	{ .data = TrailPAL,				.tag = TRAIL_PALTAG },
	{ .data = Arcade_FG_Pal,		.tag = BORDER_PALTAG },
	{ .data = TextPAL,				.tag = TEXT_PALTAG },
	{ .data = FlapPAL,				.tag = FLAP_PALTAG },
    {}
};

static const struct CompressedSpriteSheet sSpriteSheets_CreditsInterface[] =
{
    {
        .data = DigitsGFX,
        .size = 0x280,
        .tag = CREDIT_DIGITS_GFXTAG
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
    .tileTag = CREDIT_DIGITS_GFXTAG,
    .paletteTag = FLAP_PALTAG,
    .oam = &sOam_CreditDigit,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const struct CompressedSpriteSheet sSpriteSheet_Score =
{
    .data = ScoreGFX,
    .size = 0x400,
    .tag = SCORE_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_Score2 =
{
    .data = Score2GFX,
    .size = 0x400,
    .tag = SCORE2_GFXTAG,
};

static const struct OamData sOamData_Score =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Score =
{
    .tileTag = SCORE_GFXTAG,
    .paletteTag = FLAP_PALTAG,
    .oam = &sOamData_Score,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Score2 =
{
    .tileTag = SCORE2_GFXTAG,
    .paletteTag = FLAP_PALTAG,
    .oam = &sOamData_Score,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Flap =
{
    .data = FlapGFX,
    .size = 0x100,
    .tag = FLAP_GFXTAG,
};

static const struct OamData sOamData_Flap =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x8),
    .size = SPRITE_SIZE(32x8),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Flap =
{
    .tileTag = FLAP_GFXTAG,
    .paletteTag = FLAP_PALTAG,
    .oam = &sOamData_Flap,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_One =
{
    .data = OneGFX,
    .size = 0x800,
    .tag = ONE_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_Two =
{
    .data = TwoGFX,
    .size = 0x800,
    .tag = TWO_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_Three =
{
    .data = ThreeGFX,
    .size = 0x800,
    .tag = THREE_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_HiScore =
{
    .data = HiScoreGFX,
    .size = 0x800,
    .tag = HISCORE_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_Start =
{
    .data = StartGFX,
    .size = 0x800,
    .tag = START_GFXTAG,
};

static const struct CompressedSpriteSheet sSpriteSheet_GameOver =
{
    .data = GameOverGFX,
    .size = 0x800,
    .tag = GAMEOVER_GFXTAG,
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

static const struct SpriteTemplate sSpriteTemplate_One =
{
    .tileTag = ONE_GFXTAG,
    .paletteTag = TEXT_PALTAG,
    .oam = &sOamData_Text,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Two =
{
    .tileTag = TWO_GFXTAG,
    .paletteTag = TEXT_PALTAG,
    .oam = &sOamData_Text,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Three =
{
    .tileTag = THREE_GFXTAG,
    .paletteTag = TEXT_PALTAG,
    .oam = &sOamData_Text,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_HiScore =
{
    .tileTag = HISCORE_GFXTAG,
    .paletteTag = TEXT_PALTAG,
    .oam = &sOamData_Text,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Start =
{
    .tileTag = START_GFXTAG,
    .paletteTag = TEXT_PALTAG,
    .oam = &sOamData_Text,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_GameOver =
{
    .tileTag = GAMEOVER_GFXTAG,
    .paletteTag = TEXT_PALTAG,
    .oam = &sOamData_Text,
    .anims = sTitleAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Damage =
{
    .data = DamageGFX,
    .size = 0x1000,
    .tag = DAMAGE_GFXTAG,
};

static const struct OamData sOamData_Damage =
{
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
	.tileNum = 0,
    .priority = 0,
};

static const union AnimCmd sDamageAnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 2),
    ANIMCMD_FRAME(64, 2),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd sDamageAnimCmd_1[] = 
{
    ANIMCMD_FRAME(0, 20),
    ANIMCMD_FRAME(64, 20),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sDamageAnimCmds[] = {
	sDamageAnimCmd_0,
	sDamageAnimCmd_1,
};

static const struct SpriteTemplate sSpriteTemplate_Damage =
{
    .tileTag = DAMAGE_GFXTAG,
    .paletteTag = BUTTERFREE_PALTAG,
    .oam = &sOamData_Damage,
    .anims = sDamageAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Border =
{
    .data = TopLeftGFX,
    .size = 0x400,
    .tag = BORDER_GFXTAG,
};

static const struct OamData sOamData_Border =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Border =
{
    .tileTag = BORDER_GFXTAG,
    .paletteTag = BORDER_PALTAG,
    .oam = &sOamData_Border,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Border_2 =
{
    .data = BottomLeftGFX,
    .size = 0x400,
    .tag = BORDER_2_GFXTAG,
};

static const struct SpriteTemplate sSpriteTemplate_Border_2 =
{
    .tileTag = BORDER_2_GFXTAG,
    .paletteTag = BORDER_PALTAG,
    .oam = &sOamData_Border,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Trail =
{
    .data = TrailGFX,
    .size = 0x1000,
    .tag = TRAIL_GFXTAG,
};

static const struct OamData sOamData_Trail =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
	.tileNum = 0,
    .priority = 0,
};

static const union AnimCmd sTrailAnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 5),
    ANIMCMD_FRAME(16, 5),
	ANIMCMD_FRAME(32, 5),
	ANIMCMD_FRAME(48, 5),
	ANIMCMD_FRAME(64, 5),
	ANIMCMD_FRAME(80, 5),
	ANIMCMD_FRAME(96, 5),
	ANIMCMD_FRAME(112, 5),
    ANIMCMD_JUMP(0)         // Loop back to the first frame (Frame 0)
};

static const union AnimCmd *const sTrailAnimCmds[] = {
    sTrailAnimCmd_0,  
};

static const struct SpriteTemplate sSpriteTemplate_Trail =
{
    .tileTag = TRAIL_GFXTAG,
    .paletteTag = TRAIL_PALTAG,
    .oam = &sOamData_Trail,
    .anims = sTrailAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Butterfree =
{
    .data = ButterfreeGFX,
    .size = 0x1800,
    .tag = BUTTERFREE_GFXTAG,
};

static const struct OamData sOamData_Butterfree =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
	.tileNum = 0,
    .priority = 0,
};

static const union AnimCmd sButterfreeAnimCmd_0[] = 
{
	ANIMCMD_FRAME(0, 1),
    ANIMCMD_END
};

static const union AnimCmd sButterfreeAnimCmd_1[] = 
{
    ANIMCMD_FRAME(64, 10),
    ANIMCMD_FRAME(128, 10),
	ANIMCMD_FRAME(64, 10),
	ANIMCMD_FRAME(0, 10),
    ANIMCMD_END
};

static const union AnimCmd *const sButterfreeAnimCmds[] = {
	sButterfreeAnimCmd_0,
    sButterfreeAnimCmd_1,
};

static const struct SpriteTemplate sSpriteTemplate_Butterfree =
{
    .tileTag = BUTTERFREE_GFXTAG,
    .paletteTag = BUTTERFREE_PALTAG,
    .oam = &sOamData_Butterfree,
    .anims = sButterfreeAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sSpriteSheet_Butterfree_Hitbox =
{
    .data = ButterfreeHitboxGFX,
    .size = 0x80,
    .tag = BUTTERFREE_HITBOX_GFXTAG,
};

static const struct OamData sOamData_Butterfree_Hitbox =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
	.tileNum = 0,
    .priority = 0,
};

static const struct SpriteTemplate sSpriteTemplate_Butterfree_Hitbox =
{
    .tileTag = BUTTERFREE_HITBOX_GFXTAG,
    .paletteTag = BUTTERFREE_HITBOX_PALTAG,
    .oam = &sOamData_Butterfree_Hitbox,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

void StartFlappyBird(void)
{
	u8 taskId = 0;
    sFlappy = AllocZeroed(sizeof(struct FlappyBird));
    taskId = CreateTask(FadeToFlappyBirdScreen, 0);
}

static void FadeToFlappyBirdScreen(u8 taskId)
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
            SetMainCallback2(InitFlappyBirdScreen);
            DestroyTask(taskId);
        }
        break;
    }
}

static void FlappyBirdVBlankCallback(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void FlappyBirdMainCallback(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
}

static void ExitFlappyBird(void)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        FREE_AND_SET_NULL(sFlappy);
    }
}

static void StartExitFlappyBird(void)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    sFlappy->state = FLAPPY_EXIT;
}

static void ScrollX(void)
{
    // Increment the foreground scroll by 1 pixel per frame (normal speed)
    sFlappy->scroll_fg_x_frac += 256 * (3 / 2);  // 256/256 = 1 pixel per frame (normal speed)

    // Slow down the background scrolling even further (0.0625 pixels per frame)
    sFlappy->scroll_bg_x_frac += 16;   // 16/256 = 0.0625 pixels per frame (very slow)

    // Once the fractional part exceeds 256, increment the integer part and reset the fractional part
    if (sFlappy->scroll_fg_x_frac >= 256) {
        sFlappy->scroll_fg_x_frac -= 256;  // Reset fractional part for foreground
        sFlappy->scroll_fg_x_int++;       // Increment integer part for foreground
    }

    if (sFlappy->scroll_bg_x_frac >= 256) {
        sFlappy->scroll_bg_x_frac -= 256;  // Reset fractional part for background
        sFlappy->scroll_bg_x_int++;       // Increment integer part for background
    }

    // Apply the scrolling to the backgrounds (BG2 and BG3)
    SetGpuReg(REG_OFFSET_BG2HOFS, sFlappy->scroll_fg_x_int);  // Set foreground scroll (BG2)
    SetGpuReg(REG_OFFSET_BG3HOFS, sFlappy->scroll_bg_x_int);  // Set background scroll (BG3)

    // Wrap around the scroll once it exceeds 256 (screen width)
    if (sFlappy->scroll_fg_x_int >= 256) {
        sFlappy->scroll_fg_x_int = 0; // Reset if over 256 (horizontal screen width)
    }
    if (sFlappy->scroll_bg_x_int >= 256) {
        sFlappy->scroll_bg_x_int = 0; // Reset if over 256 (horizontal screen width)
    }
}

static void SetCreditDigits(u16 num)
{
    u8 i;
    u16 d = 1000;  // Start with the thousands place

    for (i = 0; i < 4; i++)  // Always show 4 digits
    {
        u8 digit = num / d;

        // Show the digit (all digits are visible)
        gSprites[sFlappy->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].invisible = FALSE;

        // If it's a smaller number, show 0 for the higher place values
        if (i == 0 && num < 1000) {
            digit = 0;  // Force 0 for the thousands place if the number is less than 1000
        }

        // Set the tileNum based on the current digit
        gSprites[sFlappy->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].oam.tileNum =
            gSprites[sFlappy->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].sheetTileStart + (digit * 2);

        gSprites[sFlappy->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].oam.priority = 0;

        // Reduce num for the next digit
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
        sFlappy->CreditSpriteIds[i + SPR_CREDIT_DIGITS] = CreateSprite(&sSpriteTemplate_CreditDigit, i * 6 + 72, 144, 0);
		gSprites[sFlappy->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].oam.priority = 0;
    }
}

static void SetScoreDigits(u16 num)
{
    u8 i;
    u16 d = 1000;  // Start with the thousands place

    for (i = 0; i < 4; i++)  // Always show 4 digits
    {
        u8 digit = num / d;

        // Show the digit (all digits are visible)
        gSprites[sFlappy->ScoreSpriteIds[i + SPR_HISCORE_DIGITS]].invisible = FALSE;

        // If it's a smaller number, show 0 for the higher place values
        if (i == 0 && num < 1000) {
            digit = 0;  // Force 0 for the thousands place if the number is less than 1000
        }

        // Set the tileNum based on the current digit
        gSprites[sFlappy->ScoreSpriteIds[i + SPR_HISCORE_DIGITS]].oam.tileNum =
            gSprites[sFlappy->ScoreSpriteIds[i + SPR_HISCORE_DIGITS]].sheetTileStart + (digit * 2);

        gSprites[sFlappy->ScoreSpriteIds[i + SPR_HISCORE_DIGITS]].oam.priority = 0;

        // Reduce num for the next digit
        num = num % d;
        d = d / 10;
    }
	
    BuildOamBuffer();
}

static void CreateHiScoreSprites(void)
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
        sFlappy->ScoreSpriteIds[i + SPR_HISCORE_DIGITS] = CreateSprite(&sSpriteTemplate_CreditDigit, i * 6 + 152, 144, 0);
		gSprites[sFlappy->ScoreSpriteIds[i + SPR_HISCORE_DIGITS]].oam.priority = 0;
    }
}

static void CreateScore(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Score.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Score.size;
		s.tag = SCORE_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->ScoreSpriteId = CreateSprite(&sSpriteTemplate_Score, 80, 138, 0); 
}

static void CreateScore2(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Score2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Score2.size;
		s.tag = SCORE2_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->Score2SpriteId = CreateSprite(&sSpriteTemplate_Score2, 160, 138, 0); 
}

static void CreateFlap(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Flap.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Flap.size;
		s.tag = FLAP_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->FlapSpriteId = CreateSprite(&sSpriteTemplate_Flap, 204, 20, 0); 
}

static void CreateOne(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_One.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_One.size;
		s.tag = ONE_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->OneSpriteId = CreateSprite(&sSpriteTemplate_One, 120, 80, 0); 
}

static void CreateTwo(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Two.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Two.size;
		s.tag = TWO_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->TwoSpriteId = CreateSprite(&sSpriteTemplate_Two, 120, 80, 0); 
}

static void CreateThree(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Three.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Three.size;
		s.tag = THREE_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->ThreeSpriteId = CreateSprite(&sSpriteTemplate_Three, 120, 80, 0); 
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
	
	sFlappy->StartSpriteId = CreateSprite(&sSpriteTemplate_Start, 120, 80, 0); 
}

static void CreateHiScore(void)
{
	struct SpriteSheet s;
	
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_HiScore.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_HiScore.size;
		s.tag = HISCORE_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->HiScoreSpriteId = CreateSprite(&sSpriteTemplate_HiScore, 120, 80, 0); 
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
	
	sFlappy->GameOverSpriteId = CreateSprite(&sSpriteTemplate_GameOver, 120, 80, 0); 
}

static void CreateHitbox(void)
{
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Butterfree_Hitbox.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Butterfree_Hitbox.size;
		s.tag = BUTTERFREE_HITBOX_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->ButterfreeHitboxSpriteId = CreateSprite(&sSpriteTemplate_Butterfree_Hitbox, 40, 64, 0);
}

static void CreateBorder(void)
{
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Border.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Border.size;
		s.tag = BORDER_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->BorderSpriteId = CreateSprite(&sSpriteTemplate_Border, 32, 16, 0);
}

static void CreateBorder2(void)
{
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Border_2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Border_2.size;
		s.tag = BORDER_2_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->BorderSprite2Id = CreateSprite(&sSpriteTemplate_Border_2, 32, 16 + 128, 0);
}

static void CreateButterfree(void)
{
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Butterfree.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Butterfree.size;
		s.tag = BUTTERFREE_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->ButterfreeSpriteId = CreateSprite(&sSpriteTemplate_Butterfree, gSprites[sFlappy->ButterfreeHitboxSpriteId].x - 5, gSprites[sFlappy->ButterfreeHitboxSpriteId].y - 6, 1);
}

static void CreateDamage(void)
{
	struct SpriteSheet s;
		LoadSpritePalettes(sSpritePalettes);
        LZ77UnCompWram(sSpriteSheet_Damage.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Damage.size;
		s.tag = DAMAGE_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->DamageSpriteId = CreateSprite(&sSpriteTemplate_Damage, gSprites[sFlappy->ButterfreeHitboxSpriteId].x - 5, gSprites[sFlappy->ButterfreeHitboxSpriteId].y - 6, 1);
}

static void CreateTrail(void)
{
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Trail.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Trail.size;
		s.tag = TRAIL_GFXTAG;
		LoadSpriteSheet(&s);
	
	sFlappy->TrailSpriteId = CreateSprite(&sSpriteTemplate_Trail, gSprites[sFlappy->ButterfreeHitboxSpriteId].x - 16, gSprites[sFlappy->ButterfreeHitboxSpriteId].y, 3);
}

static void SpriteCB_FlappyDamage(struct Sprite *sprite)
{
    s16 sine;
    s16 cosine;
    s16 rotation;
    struct ObjAffineSrcData affine;
    struct OamMatrix matrix;
    
    sFlappy->DamageSpriteRotation += 8;  

	sFlappy->DamageSpriteRotation %= 360;

    rotation = sFlappy->DamageSpriteRotation;  

    sine = Sin2(rotation);  
    cosine = Cos2(rotation); 

    affine.xScale = 256;  
    affine.yScale = 256;  
    affine.rotation = rotation * 256;  

    ObjAffineSet(&affine, &matrix, 1, 2);

    SetOamMatrix(0, matrix.a, matrix.b, matrix.c, matrix.d);  

    sprite->oam.x = sprite->x;  
    sprite->oam.y = sprite->y;  
}

static void ApplyGravity(void)
{
    sFlappy->speed_y += GRAVITY;  

    sFlappy->pos_y += sFlappy->speed_y;

    if (sFlappy->pos_y > sFlappy->MIN_Y) {  
        sFlappy->speed_y = 0;  
		PlayBGM(MUS_NONE);
		PlaySE(SE_SUPER_EFFECTIVE);
		DestroySpriteAndFreeResources(&gSprites[sFlappy->TrailSpriteId]);
		DestroySpriteAndFreeResources(&gSprites[sFlappy->ButterfreeSpriteId]);
		CreateDamage();
		sFlappy->timerDelay = 15;
		sFlappy->yUpdate = 4;
		sFlappy->state = FLAPPY_STOP;
    }

    if (sFlappy->pos_y < sFlappy->MAX_Y) {
        sFlappy->speed_y = 0;  
		PlayBGM(MUS_NONE);
		PlaySE(SE_SUPER_EFFECTIVE);
		DestroySpriteAndFreeResources(&gSprites[sFlappy->TrailSpriteId]);
		DestroySpriteAndFreeResources(&gSprites[sFlappy->ButterfreeSpriteId]);
		CreateDamage();
		sFlappy->timerDelay = 15;
		sFlappy->yUpdate = 4;
		sFlappy->state = FLAPPY_STOP;
    }

    if (sFlappy->speed_y > 4) {  
        sFlappy->speed_y = 4;
    }
}

static void Flap(void)
{
	PlaySE(SE_M_WING_ATTACK);
	if (gSprites[sFlappy->ButterfreeSpriteId].animNum == 0) {
		gSprites[sFlappy->ButterfreeSpriteId].animNum = 1;
	}
	else {
		gSprites[sFlappy->ButterfreeSpriteId].animCmdIndex = 0;
		gSprites[sFlappy->ButterfreeSpriteId].animPaused = FALSE;
	}
    sFlappy->speed_y = FLAP_STRENGTH; 
}

void ChangeObstacle1() {
    u16* bg2Tilemap = (u16*)0x0600E000;  

    u16 tileGroups[5][20][4] = {
        // Group 1
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        },
        // Group 2
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        },
        // Group 3
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        },
        // Group 4
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        },
        // Group 5
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        }
    };

    int groupIndex = sFlappy->Obstacle1Id - 1;

    int startX = 12;  
    int startY = 0;   
	int y;
	int x;
	u16 tilemapIndex;

    for (y = 0; y < 20; ++y) {  
        for (x = 0; x < 4; ++x) { 
            tilemapIndex = (y + startY) * 32 + (x + startX);  

            bg2Tilemap[tilemapIndex] = tileGroups[groupIndex][y][x];
        }
    }
}

void ChangeObstacle2() {
    u16* bg2Tilemap = (u16*)0x0600E000;  

    u16 tileGroups[5][20][4] = {
        // Group 1
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        },
        // Group 2
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        },
        // Group 3
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        },
        // Group 4
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        },
        // Group 5
        {
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x5, 0x6, 0x7},
            {0x8, 0x9, 0xA, 0xB},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0x0, 0x0, 0x0, 0x0},
            {0xC, 0xD, 0xE, 0xF},
            {0x1, 0x10, 0x11, 0x12},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4},
            {0x1, 0x2, 0x3, 0x4}
        }
    };

    int groupIndex = sFlappy->Obstacle2Id - 1;

    int startX = 28;  
    int startY = 0;   
	int y;
	int x;
	u16 tilemapIndex;

    for (y = 0; y < 20; ++y) {  
        for (x = 0; x < 4; ++x) { 
            tilemapIndex = (y + startY) * 32 + (x + startX);  

            bg2Tilemap[tilemapIndex] = tileGroups[groupIndex][y][x];
        }
    }
}

#define SMOOTHING_FACTOR 4
#define INITIAL_SHOVE 10

void UpdateTrailSprite(s16 butterfreeX, s16 butterfreeY) {
    sFlappy->TrailoffsetX = butterfreeX - 16;  

    if (sFlappy->speed_y < 0 && sFlappy->isFlapping != 1) {
        sFlappy->TrailoffsetY = butterfreeY + INITIAL_SHOVE;
        sFlappy->isFlapping = 1;  
    }

    if (sFlappy->speed_y > 0) {
        sFlappy->TrailoffsetY = sFlappy->TrailoffsetY + ((butterfreeY - 8) - sFlappy->TrailoffsetY) / SMOOTHING_FACTOR;
		sFlappy->isFlapping = 0;
    } else {
        sFlappy->TrailoffsetY = sFlappy->TrailoffsetY + ((butterfreeY - 8) - sFlappy->TrailoffsetY) / SMOOTHING_FACTOR;
    }

    gSprites[sFlappy->TrailSpriteId].x = sFlappy->TrailoffsetX;
    gSprites[sFlappy->TrailSpriteId].y = sFlappy->TrailoffsetY;
}

static void SoundEffect(u8 sfx)
{
	switch (sfx)
	{
		case 0:
		PlaySE(SE_NOTE_C);
		sFlappy->SFX++;
		break;
		case 1:
		PlaySE(SE_NOTE_D);
		sFlappy->SFX++;
		break;
		case 2:
		PlaySE(SE_NOTE_E);
		sFlappy->SFX++;
		break;
		case 3:
		PlaySE(SE_NOTE_F);
		sFlappy->SFX++;
		break;
		case 4:
		PlaySE(SE_NOTE_G);
		sFlappy->SFX++;
		break;
		case 5:
		PlaySE(SE_NOTE_A);
		sFlappy->SFX++;
		break;
		case 6:
		PlaySE(SE_NOTE_B);
		sFlappy->SFX++;
		break;
		case 7:
		PlaySE(SE_NOTE_C_HIGH);
		sFlappy->SFX = 0;
		break;
	}
}

static void FlappyBirdMain(u8 taskId)
{
	
	switch (sFlappy->state)
	{
		case FLAPPY_INIT:
			if ((sFlappy->timer == 5) && (sFlappy->timerDelay == 75)) // Butterfree
			{
				PlayCry_Normal(SPECIES_BUTTERFREE, 0);
				gSprites[sFlappy->ButterfreeSpriteId].animNum = 1;
				sFlappy->timerDelay--;
			}
			if ((sFlappy->timer == 4) && (sFlappy->timerDelay == 60)) // 3
			{
				CreateThree();
				PlaySE(SE_CONTEST_PLACE);
				sFlappy->timerDelay--;
			}
			else if ((sFlappy->timer == 3) && (sFlappy->timerDelay == 60)) // 2
			{
				DestroySpriteAndFreeResources(&gSprites[sFlappy->ThreeSpriteId]);
				CreateTwo();
				PlaySE(SE_CONTEST_PLACE);
				sFlappy->timerDelay--;
			}
			else if ((sFlappy->timer == 2) && (sFlappy->timerDelay == 60)) // 1
			{
				DestroySpriteAndFreeResources(&gSprites[sFlappy->TwoSpriteId]);
				CreateOne();
				PlaySE(SE_CONTEST_PLACE);
				sFlappy->timerDelay--;
			}
			else if ((sFlappy->timer == 1) && (sFlappy->timerDelay == 60)) // Go
			{
				DestroySpriteAndFreeResources(&gSprites[sFlappy->OneSpriteId]);
				CreateStart();
				PlaySE(SE_POKENAV_ON);
				sFlappy->timerDelay--;
			}
			else if ((sFlappy->timer > 0) && (sFlappy->timerDelay > 0))
			{
				sFlappy->timerDelay--;
			}
			else if ((sFlappy->timer > 1) && (sFlappy->timerDelay == 0))
			{
				sFlappy->timer--;
				sFlappy->timerDelay = 60;
			}
			else if ((sFlappy->timer == 1) && (sFlappy->timerDelay == 0))
			{
				sFlappy->timer--;
			}
			else if ((sFlappy->timer == 0) && (sFlappy->timerDelay == 0))
			{
				DestroySpriteAndFreeResources(&gSprites[sFlappy->StartSpriteId]);
				gSprites[sFlappy->TrailSpriteId].invisible = FALSE;
				PlayBGM(MUS_RG_CELADON);
				sFlappy->state = FLAPPY_INPUT;
			}
			break;
		case FLAPPY_INPUT:
		
            ScrollX();
			sFlappy->bg2ScrollX = sFlappy->scroll_fg_x_int;
			
			if (sFlappy->bg2ScrollX == 126) // Change Obstacle 1
			{
				sFlappy->Obstacle1Id = (Random() % 5) + 1;
				ChangeObstacle1();
			}
			
			if (sFlappy->bg2ScrollX == 252) // Change Obstacle 2
			{
				sFlappy->Obstacle2Id = (Random() % 5) + 1;
				ChangeObstacle2();
			}
			
			if ((sFlappy->bg2ScrollX > 48) && (sFlappy->bg2ScrollX < 97)) // Within Collision Range
			{
				if (sFlappy->Obstacle1Id == 1)
				{
					sFlappy->MAX_Y = 26;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else if (sFlappy->Obstacle1Id == 2)
				{
					sFlappy->MAX_Y = 34;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else if (sFlappy->Obstacle1Id == 3)
				{
					sFlappy->MAX_Y = 42;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else if (sFlappy->Obstacle1Id == 4)
				{
					sFlappy->MAX_Y = 50;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else if (sFlappy->Obstacle1Id == 5)
				{
					sFlappy->MAX_Y = 58;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else
				{
					sFlappy->MAX_Y = MAX_Y_POSITION;
					sFlappy->MIN_Y = MIN_Y_POSITION;
				}
			}
			else if ((sFlappy->bg2ScrollX > 176) && (sFlappy->bg2ScrollX < 224)) // Within Collision Range 2
			{
				if (sFlappy->Obstacle2Id == 1)
				{
					sFlappy->MAX_Y = 26;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else if (sFlappy->Obstacle2Id == 2)
				{
					sFlappy->MAX_Y = 34;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else if (sFlappy->Obstacle2Id == 3)
				{
					sFlappy->MAX_Y = 42;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else if (sFlappy->Obstacle2Id == 4)
				{
					sFlappy->MAX_Y = 50;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else if (sFlappy->Obstacle2Id == 5)
				{
					sFlappy->MAX_Y = 58;
					sFlappy->MIN_Y = sFlappy->MAX_Y + 42;
				}
				else
				{
					sFlappy->MAX_Y = MAX_Y_POSITION;
					sFlappy->MIN_Y = MIN_Y_POSITION;
				}
			}
			else
			{
				sFlappy->MAX_Y = MAX_Y_POSITION;
				sFlappy->MIN_Y = MIN_Y_POSITION;
			}

            if (gMain.newKeys & A_BUTTON) {
                Flap();  
            }

			if ((sFlappy->bg2ScrollX == 225) || ((sFlappy->bg2ScrollX == 98) && (sFlappy->Obstacle1Id != 0))) // Point
			{
				SoundEffect(sFlappy->SFX);
				if (VarGet(VAR_FLIP_WINNINGS) < 9998) {
					VarSet(VAR_FLIP_WINNINGS, VarGet(VAR_FLIP_WINNINGS) + 2); }
				else
				{
					VarSet(VAR_FLIP_WINNINGS, 9999);
				}
				if (sFlappy->Points < 9999) {
				sFlappy->Points++; }
				SetCreditDigits(sFlappy->Points);
			}

            gSprites[sFlappy->ButterfreeHitboxSpriteId].y = sFlappy->pos_y;
			gSprites[sFlappy->ButterfreeSpriteId].x = gSprites[sFlappy->ButterfreeHitboxSpriteId].x - 5;
			gSprites[sFlappy->ButterfreeSpriteId].y = gSprites[sFlappy->ButterfreeHitboxSpriteId].y - 6;

            if (sFlappy->delay == 0) {
				UpdateTrailSprite(gSprites[sFlappy->ButterfreeHitboxSpriteId].x, gSprites[sFlappy->ButterfreeHitboxSpriteId].y);
				ApplyGravity();
				sFlappy->delay = 1;
			}
			else {
				sFlappy->delay--;
			}

            break;
		case FLAPPY_STOP:
			if (sFlappy->timerDelay > 0)
			{
				if (sFlappy->yUpdate == 4)
				{
					gSprites[sFlappy->DamageSpriteId].y -= 1;
					sFlappy->yUpdate = 2;
				}
				else if (sFlappy->yUpdate == 2)
				{
					gSprites[sFlappy->DamageSpriteId].y += sFlappy->yUpdate;
					sFlappy->yUpdate = 12;
				}
				else if (sFlappy->yUpdate == 12)
				{
					gSprites[sFlappy->DamageSpriteId].y -= 2;
					sFlappy->yUpdate = 2;
				}
				sFlappy->timerDelay--;
			}
			else if ((sFlappy->timerDelay == 0) && (sFlappy->RotateToggle == 0))
			{
				gSprites[sFlappy->DamageSpriteId].animNum = 1;
				gSprites[sFlappy->DamageSpriteId].animCmdIndex = 0;
				sFlappy->RotateToggle = 1;
			}
			else if ((sFlappy->timerDelay == 0) && (sFlappy->RotateToggle == 1))
			{
				sFlappy->DamageSpriteY = gSprites[sFlappy->DamageSpriteId].y;
				gSprites[sFlappy->DamageSpriteId].callback = SpriteCB_FlappyDamage;
				AnimateSprites();
				PlaySE(SE_FAINT);
				sFlappy->state = FLAPPY_ROTATION;
			}
			break;
		case FLAPPY_ROTATION:
			if ((sFlappy->jumping != 1) && (sFlappy->jumpHeight == 0)) {
				sFlappy->jumping = 1;
				sFlappy->jumpHeight = 20;
				
				sFlappy->jumpSpeed = 0;
				sFlappy->fallSpeed = 0;
				
				sFlappy->velocity = 0;

				sFlappy->originalY = sFlappy->DamageSpriteY;
			}

			if (sFlappy->jumping == 1) {
				sFlappy->velocity += 64;

				sFlappy->DamageSpriteY -= sFlappy->velocity / 256;

				if (sFlappy->DamageSpriteY < 0) {
					sFlappy->DamageSpriteY = 0;
					sFlappy->jumping = 0;
					sFlappy->velocity = 0;
				}

				if (sFlappy->DamageSpriteY <= (sFlappy->originalY - sFlappy->jumpHeight) && sFlappy->DamageSpriteY > 0) {
					sFlappy->jumping = 0;
					sFlappy->velocity = 0;
				}
			}

			else {
				sFlappy->fallSpeed += 128;
				
				sFlappy->DamageSpriteY += sFlappy->fallSpeed / 256;

				if (sFlappy->DamageSpriteY > DISPLAY_HEIGHT + 15) {
					sFlappy->DamageSpriteY = DISPLAY_HEIGHT + 16;
					DestroySpriteAndFreeResources(&gSprites[sFlappy->DamageSpriteId]);
					sFlappy->timerDelay = 140;
					PlayFanfare(MUS_TOO_BAD);
					CreateGameOver();
					sFlappy->state = FLAPPY_GAMEOVER;
				}
			}

			gSprites[sFlappy->DamageSpriteId].y = sFlappy->DamageSpriteY;

			break;

		case FLAPPY_GAMEOVER:
			gSprites[sFlappy->GameOverSpriteId].x = 120;
			gSprites[sFlappy->GameOverSpriteId].y = 80;
			if (sFlappy->timerDelay == 0)
			{
				if (sFlappy->Points > VarGet(VAR_FLAPPY_HISCORE))
				{
					PlayFanfare(MUS_LEVEL_UP);
					VarSet(VAR_FLAPPY_HISCORE, sFlappy->Points);
					sFlappy->timerDelay = 100;
					DestroySpriteAndFreeResources(&gSprites[sFlappy->GameOverSpriteId]);
					CreateHiScore();
					sFlappy->state = FLAPPY_HISCORE;
				}
				else
				{
				StartExitFlappyBird();
				}
			}
			else
			{
				sFlappy->timerDelay--;
			}
			break;
		case FLAPPY_HISCORE:
			if (sFlappy->timerDelay == 0)
			{
				StartExitFlappyBird();
			}
			else
			{
				sFlappy->timerDelay--;
			}
			break;
		case FLAPPY_EXIT:
			ExitFlappyBird();
			break;	
	}
}

static void InitFlappyBirdScreen(void)
{
    SetVBlankCallback(NULL);
    ResetAllBgsCoordinates();
    ResetVramOamAndBgCntRegs();
    ResetBgsAndClearDma3BusyFlags(0);
    ResetTempTileDataBuffers();
    
    InitBgsFromTemplates(0, sFlappyBGtemplates, ARRAY_COUNT(sFlappyBGtemplates));

    SetBgTilemapBuffer(FlappyBird_BG, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(FlappyBird_FG, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(Arcade_BG, AllocZeroed(BG_SCREEN_SIZE));
    //SetBgTilemapBuffer(FlappyBird_TEXT_MENUS, AllocZeroed(BG_SCREEN_SIZE));

	ResetPaletteFade();

	DecompressAndLoadBgGfxUsingHeap(Arcade_BG, Arcade_FG_Img, 0x600, 0, 0);
	CopyToBgTilemapBuffer(Arcade_BG, Arcade_FG_Tilemap, 0, 0);
	LoadPalette(Arcade_FG_Pal, 0x20, sizeof(Arcade_FG_Pal));
	CopyBgTilemapBufferToVram(Arcade_BG);

	DecompressAndLoadBgGfxUsingHeap(FlappyBird_FG, FlappyBird_FG_Img, 0x600, 0, 0);
	CopyToBgTilemapBuffer(FlappyBird_FG, FlappyBird_FG_Tilemap, 0, 0);
	LoadPalette(FlappyBird_FG_Pal, 0, sizeof(FlappyBird_FG_Pal));
	CopyBgTilemapBufferToVram(FlappyBird_FG);

	DecompressAndLoadBgGfxUsingHeap(FlappyBird_BG, FlappyBird_BG_Img, 0x1000, 0, 0);
	CopyToBgTilemapBuffer(FlappyBird_BG, FlappyBird_BG_Tilemap, 0, 0);
	LoadPalette(FlappyBird_BG_Pal, 0x10, sizeof(FlappyBird_BG_Pal));
	CopyBgTilemapBufferToVram(FlappyBird_BG);

	// Sprite Stuff
    ResetSpriteData();
    FreeAllSpritePalettes();
    LoadSpritePalettes(sSpritePalettes);
	CreateHitbox();
	CreateBorder();
	CreateBorder2();
	CreateFlap();
	CreateCreditSprites();
	SetCreditDigits(0);
	CreateHiScoreSprites();
	SetScoreDigits(VarGet(VAR_FLAPPY_HISCORE));
	CreateScore();
	CreateScore2();
	CreateButterfree();
	CreateTrail();
	gSprites[sFlappy->TrailSpriteId].invisible = TRUE;
	
	sFlappy->pos_y = 80;
    sFlappy->speed_y = 0;
    sFlappy->flap_strength = FLAP_STRENGTH;
	sFlappy->MIN_Y = 140;
	sFlappy->MAX_Y = 10;
	sFlappy->Obstacle2Id = 0;
	sFlappy->Obstacle2Id = 1;
	VarSet(VAR_FLIP_WINNINGS, 0);
	sFlappy->timer = 5;
	sFlappy->timerDelay = 75;

    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_BG2_ON | DISPCNT_BG3_ON);
	SetGpuReg(REG_OFFSET_BG2CNT, 0x1800);  // Set BG2 in Mode 2
	SetGpuReg(REG_OFFSET_BG3CNT, 0x1800);  // Set BG3 in Mode 2

    ShowBg(FlappyBird_BG);
    ShowBg(FlappyBird_FG);
    ShowBg(Arcade_BG);
    //ShowBg(FlappyBird_TEXT_MENUS);  // If menus should appear over the game, this can be shown

    BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(FlappyBirdVBlankCallback);
    SetMainCallback2(FlappyBirdMainCallback);
    CreateTask(FlappyBirdMain, 1);
}
