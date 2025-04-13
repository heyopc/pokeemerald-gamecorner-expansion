//#include "game_corner_blackjack.h"
#include "game_corner_gacha.h"
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
    GACHA_STATE_INIT,
    GACHA_STATE_PROCESS_INPUT,
    GACHA_STATE_COMPLETED_WAIT_FOR_SOUND,
    GACHA_STATE_PROCESS_COMPLETED_INPUT,
    GACHA_STATE_START_EXIT,
    GACHA_STATE_EXIT,
	STATE_INIT_A,
	STATE_TIMER_1,
	STATE_TWIST,
	STATE_TIMER_2,
	STATE_INIT_GIVE,
	STATE_SHAKE_1,
	STATE_TIMER_3,
	STATE_INIT_SHAKE_2,
	STATE_SHAKE_2,
	STATE_TIMER_4,
	STATE_INIT_SHAKE_3,
	STATE_TIMER_5,
	STATE_GIVE,
	STATE_FADE,
	STATE_POKEBALL_INIT,
	STATE_POKEBALL_PROCESS,
	STATE_POKEBALL_ARRIVE,
    STATE_FADE_POKEBALL_TO_NORMAL,
    STATE_POKEBALL_ARRIVE_WAIT,
    STATE_SHOW_NEW_MON,
    STATE_NEW_MON_MSG,
	NEW_1,
	NEW_2,
	NEW_3,
	NEW_4,
	NEW_5,
	NEW_6,
};

enum {
    SPR_CREDIT_DIG_1,
    SPR_CREDIT_DIG_10,
    SPR_CREDIT_DIG_100,
    SPR_CREDIT_DIG_1000,
};

enum {
    SPR_PLAYER_DIG_1,
    SPR_PLAYER_DIG_10,
	SPR_PLAYER_DIG_100,
	SPR_PLAYER_DIG_1000,
};

#define SPR_CREDIT_DIGITS SPR_CREDIT_DIG_1
#define SPR_PLAYER_DIGITS SPR_PLAYER_DIG_1

#define MAX_SPRITES_CREDIT 4
#define MAX_SPRITES_PLAYER 4

struct Gacha {
	u8 state;
	u8 GachaId;
	u8 KnobSpriteId;
	u8 DigitalTextSpriteId;
	u8 LotteryJPNspriteId;
	u8 CreditSpriteIds[MAX_SPRITES_CREDIT];
	u8 PlayerSpriteIds[MAX_SPRITES_PLAYER];	
	u8 CreditMenu1Id;
	u8 CreditMenu2Id;
	u8 PokemonOne;
	u8 PokemonTwo;
	u8 PokemonOneSpriteId;
	u8 PokemonTwoSpriteId;
	u8 PokemonThreeSpriteId;
	u8 Odds; // Chance of new Pokemon
	u8 ArrowsSpriteId;
	u8 CTAspriteId;
	u8 exitToggle;
	u16 wager;
	u8 cursorPosition;
	u8 Trigger;
	u8 Basic_CommonMax; // Total Number of Pokemon in each array
	u8 Basic_UncommonMax;
	u8 Basic_RareMax;
	u8 Basic_UltraRareMax;
	u8 Great_CommonMax;
	u8 Great_UncommonMax;
	u8 Great_RareMax;
	u8 Great_UltraRareMax;
	u8 Ultra_CommonMax;
	u8 Ultra_UncommonMax;
	u8 Ultra_RareMax;
	u8 Ultra_UltraRareMax;
	u8 Master_CommonMax;
	u8 Master_UncommonMax;
	u8 Master_RareMax;
	u8 Master_UltraRareMax;
	u8 Rarity; // 0 = Common, 1 = Uncommon, 2 = Rare, 3 = Ultra Rare
	u8 Basic_Common_Owned; // How Many Pokemon in the array does the Player Own
	u8 Basic_Uncommon_Owned;
	u8 Basic_Rare_Owned;
	u8 Basic_UltraRare_Owned;
	u8 Great_Common_Owned;
	u8 Great_Uncommon_Owned;
	u8 Great_Rare_Owned;
	u8 Great_UltraRare_Owned;
	u8 Ultra_Common_Owned;
	u8 Ultra_Uncommon_Owned;
	u8 Ultra_Rare_Owned;
	u8 Ultra_UltraRare_Owned;
	u8 Master_Common_Owned;
	u8 Master_Uncommon_Owned;
	u8 Master_Rare_Owned;
	u8 Master_UltraRare_Owned;
	u16 Basic_Total_Owned;
	u16 Great_Total_Owned;
	u16 Ultra_Total_Owned;
	u16 Master_Total_Owned;
	u16 Basic_Total_Max;
	u16 Great_Total_Max;
	u16 Ultra_Total_Max;
	u16 Master_Total_Max;
	u8 commonChance;
	u8 uncommonChance;
	u8 rareChance;
	u8 ultraRareChance;
	u8 IsNewPokemon;
	u16 Temp_Total;
	u16 CalculatedSpecies;
	u8 bouncingPokeballSpriteId;
	u8 timer;
	u8 monSpriteId;
	u32 waitTimer;
	u8 gachaState;
	u8 Input;
	u8 textColor[3];
	struct Pokemon GachaMon;
};	

static EWRAM_DATA struct {
    struct Pokemon tempMon; // Used as a temp variable when swapping Pokémon
    u32 timer;
    u32 monPersonalities[2];
    u8 filler_70[2];
    u8 playerFinishStatus;
    u8 partnerFinishStatus;
    u16 linkData[10];
    u8 linkTimeoutZero1;
    u8 linkTimeoutZero2;
    u16 linkTimeoutTimer;
    u16 neverRead_8C;
    u8 monSpriteIds[2];
    u8 connectionSpriteId1; // Multi-purpose sprite ids used during the transfer sequence
    u8 connectionSpriteId2;
    u8 cableEndSpriteId;
    u8 scheduleLinkTransfer;
    u16 state;
    u8 filler_96[0x3C];
    u8 releasePokeballSpriteId;
    u8 bouncingPokeballSpriteId;
    u16 texX;
    u16 texY;
    u16 neverRead_D8;
    u16 neverRead_DA;
    u16 scrX;
    u16 scrY;
    s16 bg1vofs;
    s16 bg1hofs;
    s16 bg2vofs;
    s16 bg2hofs;
    u16 sXY;
    u16 gbaScale;
    u16 alpha;
    bool8 isLinkTrade;
    u16 monSpecies[2];
    u16 cachedMapMusic;
    u8 textColors[3];
    u8 filler_F9;
    bool8 isCableTrade;
    u8 wirelessWinLeft;
    u8 wirelessWinTop;
    u8 wirelessWinRight;
    u8 wirelessWinBottom;
} *sTradeAnim = NULL;

extern const u8 gText_FromGacha[];
extern const u8 gText_NicknameGacha[];

static const s8 sTradeBallVerticalVelocityTable[] =
{
     0,  0,  1,  0,  1,  0,  1,  1,  1,
     1,  2,  2,  2,  2,  3,  3,  3,  3,
     4,  4,  4,  4, -4, -4, -4, -3, -3,
    -3, -3, -2, -2, -2, -2, -1, -1, -1,
    -1,  0, -1,  0, -1,  0,  0,  0,  0,
     0,  1,  0,  1,  0,  1,  1,  1,  1,
     2,  2,  2,  2,  3,  3,  3,  3,  4,
     4,  4,  4, -4, -3, -3, -2, -2, -1,
    -1, -1,  0, -1,  0,  0,  0,  0,  0,
     0,  1,  0,  1,  1,  1,  2,  2,  3,
     3,  4, -4, -3, -2, -1, -1, -1,  0,
     0,  0,  0,  1,  0,  1,  1,  2,  3
};

static EWRAM_DATA struct Gacha *sGacha = NULL;
static EWRAM_DATA u8 sTextWindowId = 0;

static void FadeToGachaScreen(u8 taskId);
static void InitGachaScreen(void);
static void GachaVBlankCallback(void);
static void SpriteCB_BouncingPokeball(struct Sprite *);
static void SpriteCB_BouncingPokeballArrive(struct Sprite *);
static void InitTradeScreen(void);
static void Process_A(void);
static void EggHatchPrintMessage(u8, u8 *, u8, u8, u8);

static const u8 sMessageText[] = _("NEW POKéMON : {STR_VAR_1}%");

static void SpriteCB_Null(struct Sprite *sprite)
{
}

#define SPECIES_TO_NATIONAL(name)   [SPECIES_##name - 1] = NATIONAL_DEX_##name

static const u16 sSpeciesToNationalPokedexNum[NUM_SPECIES - 1] =
{
    SPECIES_TO_NATIONAL(BULBASAUR),
    SPECIES_TO_NATIONAL(IVYSAUR),
    SPECIES_TO_NATIONAL(VENUSAUR),
    SPECIES_TO_NATIONAL(CHARMANDER),
    SPECIES_TO_NATIONAL(CHARMELEON),
    SPECIES_TO_NATIONAL(CHARIZARD),
    SPECIES_TO_NATIONAL(SQUIRTLE),
    SPECIES_TO_NATIONAL(WARTORTLE),
    SPECIES_TO_NATIONAL(BLASTOISE),
    SPECIES_TO_NATIONAL(CATERPIE),
    SPECIES_TO_NATIONAL(METAPOD),
    SPECIES_TO_NATIONAL(BUTTERFREE),
    SPECIES_TO_NATIONAL(WEEDLE),
    SPECIES_TO_NATIONAL(KAKUNA),
    SPECIES_TO_NATIONAL(BEEDRILL),
    SPECIES_TO_NATIONAL(PIDGEY),
    SPECIES_TO_NATIONAL(PIDGEOTTO),
    SPECIES_TO_NATIONAL(PIDGEOT),
    SPECIES_TO_NATIONAL(RATTATA),
    SPECIES_TO_NATIONAL(RATICATE),
    SPECIES_TO_NATIONAL(SPEAROW),
    SPECIES_TO_NATIONAL(FEAROW),
    SPECIES_TO_NATIONAL(EKANS),
    SPECIES_TO_NATIONAL(ARBOK),
    SPECIES_TO_NATIONAL(PIKACHU),
    SPECIES_TO_NATIONAL(RAICHU),
    SPECIES_TO_NATIONAL(SANDSHREW),
    SPECIES_TO_NATIONAL(SANDSLASH),
    SPECIES_TO_NATIONAL(NIDORAN_F),
    SPECIES_TO_NATIONAL(NIDORINA),
    SPECIES_TO_NATIONAL(NIDOQUEEN),
    SPECIES_TO_NATIONAL(NIDORAN_M),
    SPECIES_TO_NATIONAL(NIDORINO),
    SPECIES_TO_NATIONAL(NIDOKING),
    SPECIES_TO_NATIONAL(CLEFAIRY),
    SPECIES_TO_NATIONAL(CLEFABLE),
    SPECIES_TO_NATIONAL(VULPIX),
    SPECIES_TO_NATIONAL(NINETALES),
    SPECIES_TO_NATIONAL(JIGGLYPUFF),
    SPECIES_TO_NATIONAL(WIGGLYTUFF),
    SPECIES_TO_NATIONAL(ZUBAT),
    SPECIES_TO_NATIONAL(GOLBAT),
    SPECIES_TO_NATIONAL(ODDISH),
    SPECIES_TO_NATIONAL(GLOOM),
    SPECIES_TO_NATIONAL(VILEPLUME),
    SPECIES_TO_NATIONAL(PARAS),
    SPECIES_TO_NATIONAL(PARASECT),
    SPECIES_TO_NATIONAL(VENONAT),
    SPECIES_TO_NATIONAL(VENOMOTH),
    SPECIES_TO_NATIONAL(DIGLETT),
    SPECIES_TO_NATIONAL(DUGTRIO),
    SPECIES_TO_NATIONAL(MEOWTH),
    SPECIES_TO_NATIONAL(PERSIAN),
    SPECIES_TO_NATIONAL(PSYDUCK),
    SPECIES_TO_NATIONAL(GOLDUCK),
    SPECIES_TO_NATIONAL(MANKEY),
    SPECIES_TO_NATIONAL(PRIMEAPE),
    SPECIES_TO_NATIONAL(GROWLITHE),
    SPECIES_TO_NATIONAL(ARCANINE),
    SPECIES_TO_NATIONAL(POLIWAG),
    SPECIES_TO_NATIONAL(POLIWHIRL),
    SPECIES_TO_NATIONAL(POLIWRATH),
    SPECIES_TO_NATIONAL(ABRA),
    SPECIES_TO_NATIONAL(KADABRA),
    SPECIES_TO_NATIONAL(ALAKAZAM),
    SPECIES_TO_NATIONAL(MACHOP),
    SPECIES_TO_NATIONAL(MACHOKE),
    SPECIES_TO_NATIONAL(MACHAMP),
    SPECIES_TO_NATIONAL(BELLSPROUT),
    SPECIES_TO_NATIONAL(WEEPINBELL),
    SPECIES_TO_NATIONAL(VICTREEBEL),
    SPECIES_TO_NATIONAL(TENTACOOL),
    SPECIES_TO_NATIONAL(TENTACRUEL),
    SPECIES_TO_NATIONAL(GEODUDE),
    SPECIES_TO_NATIONAL(GRAVELER),
    SPECIES_TO_NATIONAL(GOLEM),
    SPECIES_TO_NATIONAL(PONYTA),
    SPECIES_TO_NATIONAL(RAPIDASH),
    SPECIES_TO_NATIONAL(SLOWPOKE),
    SPECIES_TO_NATIONAL(SLOWBRO),
    SPECIES_TO_NATIONAL(MAGNEMITE),
    SPECIES_TO_NATIONAL(MAGNETON),
    SPECIES_TO_NATIONAL(FARFETCHD),
    SPECIES_TO_NATIONAL(DODUO),
    SPECIES_TO_NATIONAL(DODRIO),
    SPECIES_TO_NATIONAL(SEEL),
    SPECIES_TO_NATIONAL(DEWGONG),
    SPECIES_TO_NATIONAL(GRIMER),
    SPECIES_TO_NATIONAL(MUK),
    SPECIES_TO_NATIONAL(SHELLDER),
    SPECIES_TO_NATIONAL(CLOYSTER),
    SPECIES_TO_NATIONAL(GASTLY),
    SPECIES_TO_NATIONAL(HAUNTER),
    SPECIES_TO_NATIONAL(GENGAR),
    SPECIES_TO_NATIONAL(ONIX),
    SPECIES_TO_NATIONAL(DROWZEE),
    SPECIES_TO_NATIONAL(HYPNO),
    SPECIES_TO_NATIONAL(KRABBY),
    SPECIES_TO_NATIONAL(KINGLER),
    SPECIES_TO_NATIONAL(VOLTORB),
    SPECIES_TO_NATIONAL(ELECTRODE),
    SPECIES_TO_NATIONAL(EXEGGCUTE),
    SPECIES_TO_NATIONAL(EXEGGUTOR),
    SPECIES_TO_NATIONAL(CUBONE),
    SPECIES_TO_NATIONAL(MAROWAK),
    SPECIES_TO_NATIONAL(HITMONLEE),
    SPECIES_TO_NATIONAL(HITMONCHAN),
    SPECIES_TO_NATIONAL(LICKITUNG),
    SPECIES_TO_NATIONAL(KOFFING),
    SPECIES_TO_NATIONAL(WEEZING),
    SPECIES_TO_NATIONAL(RHYHORN),
    SPECIES_TO_NATIONAL(RHYDON),
    SPECIES_TO_NATIONAL(CHANSEY),
    SPECIES_TO_NATIONAL(TANGELA),
    SPECIES_TO_NATIONAL(KANGASKHAN),
    SPECIES_TO_NATIONAL(HORSEA),
    SPECIES_TO_NATIONAL(SEADRA),
    SPECIES_TO_NATIONAL(GOLDEEN),
    SPECIES_TO_NATIONAL(SEAKING),
    SPECIES_TO_NATIONAL(STARYU),
    SPECIES_TO_NATIONAL(STARMIE),
    SPECIES_TO_NATIONAL(MR_MIME),
    SPECIES_TO_NATIONAL(SCYTHER),
    SPECIES_TO_NATIONAL(JYNX),
    SPECIES_TO_NATIONAL(ELECTABUZZ),
    SPECIES_TO_NATIONAL(MAGMAR),
    SPECIES_TO_NATIONAL(PINSIR),
    SPECIES_TO_NATIONAL(TAUROS),
    SPECIES_TO_NATIONAL(MAGIKARP),
    SPECIES_TO_NATIONAL(GYARADOS),
    SPECIES_TO_NATIONAL(LAPRAS),
    SPECIES_TO_NATIONAL(DITTO),
    SPECIES_TO_NATIONAL(EEVEE),
    SPECIES_TO_NATIONAL(VAPOREON),
    SPECIES_TO_NATIONAL(JOLTEON),
    SPECIES_TO_NATIONAL(FLAREON),
    SPECIES_TO_NATIONAL(PORYGON),
    SPECIES_TO_NATIONAL(OMANYTE),
    SPECIES_TO_NATIONAL(OMASTAR),
    SPECIES_TO_NATIONAL(KABUTO),
    SPECIES_TO_NATIONAL(KABUTOPS),
    SPECIES_TO_NATIONAL(AERODACTYL),
    SPECIES_TO_NATIONAL(SNORLAX),
    SPECIES_TO_NATIONAL(ARTICUNO),
    SPECIES_TO_NATIONAL(ZAPDOS),
    SPECIES_TO_NATIONAL(MOLTRES),
    SPECIES_TO_NATIONAL(DRATINI),
    SPECIES_TO_NATIONAL(DRAGONAIR),
    SPECIES_TO_NATIONAL(DRAGONITE),
    SPECIES_TO_NATIONAL(MEWTWO),
    SPECIES_TO_NATIONAL(MEW),
    SPECIES_TO_NATIONAL(CHIKORITA),
    SPECIES_TO_NATIONAL(BAYLEEF),
    SPECIES_TO_NATIONAL(MEGANIUM),
    SPECIES_TO_NATIONAL(CYNDAQUIL),
    SPECIES_TO_NATIONAL(QUILAVA),
    SPECIES_TO_NATIONAL(TYPHLOSION),
    SPECIES_TO_NATIONAL(TOTODILE),
    SPECIES_TO_NATIONAL(CROCONAW),
    SPECIES_TO_NATIONAL(FERALIGATR),
    SPECIES_TO_NATIONAL(SENTRET),
    SPECIES_TO_NATIONAL(FURRET),
    SPECIES_TO_NATIONAL(HOOTHOOT),
    SPECIES_TO_NATIONAL(NOCTOWL),
    SPECIES_TO_NATIONAL(LEDYBA),
    SPECIES_TO_NATIONAL(LEDIAN),
    SPECIES_TO_NATIONAL(SPINARAK),
    SPECIES_TO_NATIONAL(ARIADOS),
    SPECIES_TO_NATIONAL(CROBAT),
    SPECIES_TO_NATIONAL(CHINCHOU),
    SPECIES_TO_NATIONAL(LANTURN),
    SPECIES_TO_NATIONAL(PICHU),
    SPECIES_TO_NATIONAL(CLEFFA),
    SPECIES_TO_NATIONAL(IGGLYBUFF),
    SPECIES_TO_NATIONAL(TOGEPI),
    SPECIES_TO_NATIONAL(TOGETIC),
    SPECIES_TO_NATIONAL(NATU),
    SPECIES_TO_NATIONAL(XATU),
    SPECIES_TO_NATIONAL(MAREEP),
    SPECIES_TO_NATIONAL(FLAAFFY),
    SPECIES_TO_NATIONAL(AMPHAROS),
    SPECIES_TO_NATIONAL(BELLOSSOM),
    SPECIES_TO_NATIONAL(MARILL),
    SPECIES_TO_NATIONAL(AZUMARILL),
    SPECIES_TO_NATIONAL(SUDOWOODO),
    SPECIES_TO_NATIONAL(POLITOED),
    SPECIES_TO_NATIONAL(HOPPIP),
    SPECIES_TO_NATIONAL(SKIPLOOM),
    SPECIES_TO_NATIONAL(JUMPLUFF),
    SPECIES_TO_NATIONAL(AIPOM),
    SPECIES_TO_NATIONAL(SUNKERN),
    SPECIES_TO_NATIONAL(SUNFLORA),
    SPECIES_TO_NATIONAL(YANMA),
    SPECIES_TO_NATIONAL(WOOPER),
    SPECIES_TO_NATIONAL(QUAGSIRE),
    SPECIES_TO_NATIONAL(ESPEON),
    SPECIES_TO_NATIONAL(UMBREON),
    SPECIES_TO_NATIONAL(MURKROW),
    SPECIES_TO_NATIONAL(SLOWKING),
    SPECIES_TO_NATIONAL(MISDREAVUS),
    SPECIES_TO_NATIONAL(UNOWN),
    SPECIES_TO_NATIONAL(WOBBUFFET),
    SPECIES_TO_NATIONAL(GIRAFARIG),
    SPECIES_TO_NATIONAL(PINECO),
    SPECIES_TO_NATIONAL(FORRETRESS),
    SPECIES_TO_NATIONAL(DUNSPARCE),
    SPECIES_TO_NATIONAL(GLIGAR),
    SPECIES_TO_NATIONAL(STEELIX),
    SPECIES_TO_NATIONAL(SNUBBULL),
    SPECIES_TO_NATIONAL(GRANBULL),
    SPECIES_TO_NATIONAL(QWILFISH),
    SPECIES_TO_NATIONAL(SCIZOR),
    SPECIES_TO_NATIONAL(SHUCKLE),
    SPECIES_TO_NATIONAL(HERACROSS),
    SPECIES_TO_NATIONAL(SNEASEL),
    SPECIES_TO_NATIONAL(TEDDIURSA),
    SPECIES_TO_NATIONAL(URSARING),
    SPECIES_TO_NATIONAL(SLUGMA),
    SPECIES_TO_NATIONAL(MAGCARGO),
    SPECIES_TO_NATIONAL(SWINUB),
    SPECIES_TO_NATIONAL(PILOSWINE),
    SPECIES_TO_NATIONAL(CORSOLA),
    SPECIES_TO_NATIONAL(REMORAID),
    SPECIES_TO_NATIONAL(OCTILLERY),
    SPECIES_TO_NATIONAL(DELIBIRD),
    SPECIES_TO_NATIONAL(MANTINE),
    SPECIES_TO_NATIONAL(SKARMORY),
    SPECIES_TO_NATIONAL(HOUNDOUR),
    SPECIES_TO_NATIONAL(HOUNDOOM),
    SPECIES_TO_NATIONAL(KINGDRA),
    SPECIES_TO_NATIONAL(PHANPY),
    SPECIES_TO_NATIONAL(DONPHAN),
    SPECIES_TO_NATIONAL(PORYGON2),
    SPECIES_TO_NATIONAL(STANTLER),
    SPECIES_TO_NATIONAL(SMEARGLE),
    SPECIES_TO_NATIONAL(TYROGUE),
    SPECIES_TO_NATIONAL(HITMONTOP),
    SPECIES_TO_NATIONAL(SMOOCHUM),
    SPECIES_TO_NATIONAL(ELEKID),
    SPECIES_TO_NATIONAL(MAGBY),
    SPECIES_TO_NATIONAL(MILTANK),
    SPECIES_TO_NATIONAL(BLISSEY),
    SPECIES_TO_NATIONAL(RAIKOU),
    SPECIES_TO_NATIONAL(ENTEI),
    SPECIES_TO_NATIONAL(SUICUNE),
    SPECIES_TO_NATIONAL(LARVITAR),
    SPECIES_TO_NATIONAL(PUPITAR),
    SPECIES_TO_NATIONAL(TYRANITAR),
    SPECIES_TO_NATIONAL(LUGIA),
    SPECIES_TO_NATIONAL(HO_OH),
    SPECIES_TO_NATIONAL(CELEBI),
    SPECIES_TO_NATIONAL(OLD_UNOWN_B),
    SPECIES_TO_NATIONAL(OLD_UNOWN_C),
    SPECIES_TO_NATIONAL(OLD_UNOWN_D),
    SPECIES_TO_NATIONAL(OLD_UNOWN_E),
    SPECIES_TO_NATIONAL(OLD_UNOWN_F),
    SPECIES_TO_NATIONAL(OLD_UNOWN_G),
    SPECIES_TO_NATIONAL(OLD_UNOWN_H),
    SPECIES_TO_NATIONAL(OLD_UNOWN_I),
    SPECIES_TO_NATIONAL(OLD_UNOWN_J),
    SPECIES_TO_NATIONAL(OLD_UNOWN_K),
    SPECIES_TO_NATIONAL(OLD_UNOWN_L),
    SPECIES_TO_NATIONAL(OLD_UNOWN_M),
    SPECIES_TO_NATIONAL(OLD_UNOWN_N),
    SPECIES_TO_NATIONAL(OLD_UNOWN_O),
    SPECIES_TO_NATIONAL(OLD_UNOWN_P),
    SPECIES_TO_NATIONAL(OLD_UNOWN_Q),
    SPECIES_TO_NATIONAL(OLD_UNOWN_R),
    SPECIES_TO_NATIONAL(OLD_UNOWN_S),
    SPECIES_TO_NATIONAL(OLD_UNOWN_T),
    SPECIES_TO_NATIONAL(OLD_UNOWN_U),
    SPECIES_TO_NATIONAL(OLD_UNOWN_V),
    SPECIES_TO_NATIONAL(OLD_UNOWN_W),
    SPECIES_TO_NATIONAL(OLD_UNOWN_X),
    SPECIES_TO_NATIONAL(OLD_UNOWN_Y),
    SPECIES_TO_NATIONAL(OLD_UNOWN_Z),
    SPECIES_TO_NATIONAL(TREECKO),
    SPECIES_TO_NATIONAL(GROVYLE),
    SPECIES_TO_NATIONAL(SCEPTILE),
    SPECIES_TO_NATIONAL(TORCHIC),
    SPECIES_TO_NATIONAL(COMBUSKEN),
    SPECIES_TO_NATIONAL(BLAZIKEN),
    SPECIES_TO_NATIONAL(MUDKIP),
    SPECIES_TO_NATIONAL(MARSHTOMP),
    SPECIES_TO_NATIONAL(SWAMPERT),
    SPECIES_TO_NATIONAL(POOCHYENA),
    SPECIES_TO_NATIONAL(MIGHTYENA),
    SPECIES_TO_NATIONAL(ZIGZAGOON),
    SPECIES_TO_NATIONAL(LINOONE),
    SPECIES_TO_NATIONAL(WURMPLE),
    SPECIES_TO_NATIONAL(SILCOON),
    SPECIES_TO_NATIONAL(BEAUTIFLY),
    SPECIES_TO_NATIONAL(CASCOON),
    SPECIES_TO_NATIONAL(DUSTOX),
    SPECIES_TO_NATIONAL(LOTAD),
    SPECIES_TO_NATIONAL(LOMBRE),
    SPECIES_TO_NATIONAL(LUDICOLO),
    SPECIES_TO_NATIONAL(SEEDOT),
    SPECIES_TO_NATIONAL(NUZLEAF),
    SPECIES_TO_NATIONAL(SHIFTRY),
    SPECIES_TO_NATIONAL(NINCADA),
    SPECIES_TO_NATIONAL(NINJASK),
    SPECIES_TO_NATIONAL(SHEDINJA),
    SPECIES_TO_NATIONAL(TAILLOW),
    SPECIES_TO_NATIONAL(SWELLOW),
    SPECIES_TO_NATIONAL(SHROOMISH),
    SPECIES_TO_NATIONAL(BRELOOM),
    SPECIES_TO_NATIONAL(SPINDA),
    SPECIES_TO_NATIONAL(WINGULL),
    SPECIES_TO_NATIONAL(PELIPPER),
    SPECIES_TO_NATIONAL(SURSKIT),
    SPECIES_TO_NATIONAL(MASQUERAIN),
    SPECIES_TO_NATIONAL(WAILMER),
    SPECIES_TO_NATIONAL(WAILORD),
    SPECIES_TO_NATIONAL(SKITTY),
    SPECIES_TO_NATIONAL(DELCATTY),
    SPECIES_TO_NATIONAL(KECLEON),
    SPECIES_TO_NATIONAL(BALTOY),
    SPECIES_TO_NATIONAL(CLAYDOL),
    SPECIES_TO_NATIONAL(NOSEPASS),
    SPECIES_TO_NATIONAL(TORKOAL),
    SPECIES_TO_NATIONAL(SABLEYE),
    SPECIES_TO_NATIONAL(BARBOACH),
    SPECIES_TO_NATIONAL(WHISCASH),
    SPECIES_TO_NATIONAL(LUVDISC),
    SPECIES_TO_NATIONAL(CORPHISH),
    SPECIES_TO_NATIONAL(CRAWDAUNT),
    SPECIES_TO_NATIONAL(FEEBAS),
    SPECIES_TO_NATIONAL(MILOTIC),
    SPECIES_TO_NATIONAL(CARVANHA),
    SPECIES_TO_NATIONAL(SHARPEDO),
    SPECIES_TO_NATIONAL(TRAPINCH),
    SPECIES_TO_NATIONAL(VIBRAVA),
    SPECIES_TO_NATIONAL(FLYGON),
    SPECIES_TO_NATIONAL(MAKUHITA),
    SPECIES_TO_NATIONAL(HARIYAMA),
    SPECIES_TO_NATIONAL(ELECTRIKE),
    SPECIES_TO_NATIONAL(MANECTRIC),
    SPECIES_TO_NATIONAL(NUMEL),
    SPECIES_TO_NATIONAL(CAMERUPT),
    SPECIES_TO_NATIONAL(SPHEAL),
    SPECIES_TO_NATIONAL(SEALEO),
    SPECIES_TO_NATIONAL(WALREIN),
    SPECIES_TO_NATIONAL(CACNEA),
    SPECIES_TO_NATIONAL(CACTURNE),
    SPECIES_TO_NATIONAL(SNORUNT),
    SPECIES_TO_NATIONAL(GLALIE),
    SPECIES_TO_NATIONAL(LUNATONE),
    SPECIES_TO_NATIONAL(SOLROCK),
    SPECIES_TO_NATIONAL(AZURILL),
    SPECIES_TO_NATIONAL(SPOINK),
    SPECIES_TO_NATIONAL(GRUMPIG),
    SPECIES_TO_NATIONAL(PLUSLE),
    SPECIES_TO_NATIONAL(MINUN),
    SPECIES_TO_NATIONAL(MAWILE),
    SPECIES_TO_NATIONAL(MEDITITE),
    SPECIES_TO_NATIONAL(MEDICHAM),
    SPECIES_TO_NATIONAL(SWABLU),
    SPECIES_TO_NATIONAL(ALTARIA),
    SPECIES_TO_NATIONAL(WYNAUT),
    SPECIES_TO_NATIONAL(DUSKULL),
    SPECIES_TO_NATIONAL(DUSCLOPS),
    SPECIES_TO_NATIONAL(ROSELIA),
    SPECIES_TO_NATIONAL(SLAKOTH),
    SPECIES_TO_NATIONAL(VIGOROTH),
    SPECIES_TO_NATIONAL(SLAKING),
    SPECIES_TO_NATIONAL(GULPIN),
    SPECIES_TO_NATIONAL(SWALOT),
    SPECIES_TO_NATIONAL(TROPIUS),
    SPECIES_TO_NATIONAL(WHISMUR),
    SPECIES_TO_NATIONAL(LOUDRED),
    SPECIES_TO_NATIONAL(EXPLOUD),
    SPECIES_TO_NATIONAL(CLAMPERL),
    SPECIES_TO_NATIONAL(HUNTAIL),
    SPECIES_TO_NATIONAL(GOREBYSS),
    SPECIES_TO_NATIONAL(ABSOL),
    SPECIES_TO_NATIONAL(SHUPPET),
    SPECIES_TO_NATIONAL(BANETTE),
    SPECIES_TO_NATIONAL(SEVIPER),
    SPECIES_TO_NATIONAL(ZANGOOSE),
    SPECIES_TO_NATIONAL(RELICANTH),
    SPECIES_TO_NATIONAL(ARON),
    SPECIES_TO_NATIONAL(LAIRON),
    SPECIES_TO_NATIONAL(AGGRON),
    SPECIES_TO_NATIONAL(CASTFORM),
    SPECIES_TO_NATIONAL(VOLBEAT),
    SPECIES_TO_NATIONAL(ILLUMISE),
    SPECIES_TO_NATIONAL(LILEEP),
    SPECIES_TO_NATIONAL(CRADILY),
    SPECIES_TO_NATIONAL(ANORITH),
    SPECIES_TO_NATIONAL(ARMALDO),
    SPECIES_TO_NATIONAL(RALTS),
    SPECIES_TO_NATIONAL(KIRLIA),
    SPECIES_TO_NATIONAL(GARDEVOIR),
    SPECIES_TO_NATIONAL(BAGON),
    SPECIES_TO_NATIONAL(SHELGON),
    SPECIES_TO_NATIONAL(SALAMENCE),
    SPECIES_TO_NATIONAL(BELDUM),
    SPECIES_TO_NATIONAL(METANG),
    SPECIES_TO_NATIONAL(METAGROSS),
    SPECIES_TO_NATIONAL(REGIROCK),
    SPECIES_TO_NATIONAL(REGICE),
    SPECIES_TO_NATIONAL(REGISTEEL),
    SPECIES_TO_NATIONAL(KYOGRE),
    SPECIES_TO_NATIONAL(GROUDON),
    SPECIES_TO_NATIONAL(RAYQUAZA),
    SPECIES_TO_NATIONAL(LATIAS),
    SPECIES_TO_NATIONAL(LATIOS),
    SPECIES_TO_NATIONAL(JIRACHI),
    SPECIES_TO_NATIONAL(DEOXYS),
    SPECIES_TO_NATIONAL(CHIMECHO),
};

// BG Images/Tilemaps

// Main, no shake
static const u32 Gacha_BG_Main[] = INCBIN_U32("graphics/gacha/bg_middle.4bpp.lz");
static const u8 Gacha_BG_Main_Tilemap[] = INCBIN_U8("graphics/gacha/bg_middle.bin.lz");
// Left shake
static const u32 Gacha_BG_Left[] = INCBIN_U32("graphics/gacha/bg_left.4bpp.lz");
static const u8 Gacha_BG_Left_Tilemap[] = INCBIN_U8("graphics/gacha/bg_left.bin.lz");
// Right shake
static const u32 Gacha_BG_Right[] = INCBIN_U32("graphics/gacha/bg_right.4bpp.lz");
static const u8 Gacha_BG_Right_Tilemap[] = INCBIN_U8("graphics/gacha/bg_right.bin.lz");

// Trade
static const u32 Gacha_BG_Red[] = INCBIN_U32("graphics/gacha/bg_mon.4bpp.lz");
static const u8 Gacha_BG_Red_Tilemap[] = INCBIN_U8("graphics/gacha/bg_mon.bin.lz");

// BG Palettes

// Basic
static const u16 Gacha_BG_Basic_Pal[] = INCBIN_U16("graphics/gacha/bg_basic.gbapal");
// Great
static const u16 Gacha_BG_Great_Pal[] = INCBIN_U16("graphics/gacha/bg_great.gbapal");
// Ultra
static const u16 Gacha_BG_Ultra_Pal[] = INCBIN_U16("graphics/gacha/bg_ultra.gbapal");
// Master
static const u16 Gacha_BG_Master_Pal[] = INCBIN_U16("graphics/gacha/bg_master.gbapal");

static const u16 Gacha_BG_Red_Pal[] = INCBIN_U16("graphics/gacha/bg_mon.gbapal");

// Knob Sprite Image
static const u32 Gacha_Knob[] = INCBIN_U32("graphics/gacha/knob.4bpp.lz");

// Knob Sprite Palettes

static const u16 Gacha_Knob_Pal[] = INCBIN_U16("graphics/gacha/knob.gbapal");
static const u16 Gacha_Digital_Text_Pal[] = INCBIN_U16("graphics/gacha/digital_text.gbapal");
static const u16 Gacha_Lottery_Pal[] = INCBIN_U16("graphics/gacha/lottery.gbapal");
static const u16 Gacha_press_a_Pal[] = INCBIN_U16("graphics/gacha/press_a.gbapal");

// Digital Text
static const u32 Gacha_Digital_Text[] = INCBIN_U32("graphics/gacha/digital_text.4bpp.lz");

// Title, Japanese
static const u32 Gacha_Lottery_JPN[] = INCBIN_U32("graphics/gacha/lottery_japan.4bpp.lz");

//Numbers

static const u32 gCredits_Gfx[] = INCBIN_U32("graphics/gacha/numbers.4bpp.lz");
static const u16 sCredit_Pal[] = INCBIN_U16("graphics/gacha/numbers.gbapal");

static const u32 gPlayer_Gfx[] = INCBIN_U32("graphics/gacha/input_numbers.4bpp.lz");
static const u16 sPlayer_Pal[] = INCBIN_U16("graphics/gacha/input_numbers.gbapal");

// Credits Menu

// Images

static const u32 Gacha_Menu_1[] = INCBIN_U32("graphics/gacha/menu_1.4bpp.lz");
static const u32 Gacha_Menu_2[] = INCBIN_U32("graphics/gacha/menu_2.4bpp.lz");

// Palettes

// Basic
static const u16 Gacha_Menu_Basic_Pal[] = INCBIN_U16("graphics/gacha/menu_basic.gbapal");
// Great
static const u16 Gacha_Menu_Great_Pal[] = INCBIN_U16("graphics/gacha/menu_great.gbapal");
// Ultra
static const u16 Gacha_Menu_Ultra_Pal[] = INCBIN_U16("graphics/gacha/menu_ultra.gbapal");
// Master
static const u16 Gacha_Menu_Master_Pal[] = INCBIN_U16("graphics/gacha/menu_master.gbapal");

// Basic
static const u16 Gacha_Menu2_Basic_Pal[] = INCBIN_U16("graphics/gacha/menu2_basic.gbapal");
// Great
static const u16 Gacha_Menu2_Great_Pal[] = INCBIN_U16("graphics/gacha/menu2_great.gbapal");
// Ultra
static const u16 Gacha_Menu2_Ultra_Pal[] = INCBIN_U16("graphics/gacha/menu2_ultra.gbapal");
// Master
static const u16 Gacha_Menu2_Master_Pal[] = INCBIN_U16("graphics/gacha/menu2_master.gbapal");

// Pokemon

// Belossom
static const u32 BelossomGFX[] = INCBIN_U32("graphics/gacha/belossom.4bpp.lz");
static const u16 BelossomPAL[] = INCBIN_U16("graphics/gacha/belossom.gbapal");

// Phanpy
static const u32 PhanpyGFX[] = INCBIN_U32("graphics/gacha/phanpy.4bpp.lz");
static const u16 PhanpyPal[] = INCBIN_U16("graphics/gacha/phanpy.gbapal");

// Teddiursa
static const u32 TeddiursaGFX[] = INCBIN_U32("graphics/gacha/teddiursa.4bpp.lz");
static const u16 TeddiursaPAL[] = INCBIN_U16("graphics/gacha/teddiursa.gbapal");

// Elekid
static const u32 ElekidGFX[] = INCBIN_U32("graphics/gacha/elekid.4bpp.lz");
static const u16 ElekidPAL[] = INCBIN_U16("graphics/gacha/elekid.gbapal");

// Hoppip
static const u32 HoppipGFX[] = INCBIN_U32("graphics/gacha/hoppip.4bpp.lz");
static const u16 HoppipPAL[] = INCBIN_U16("graphics/gacha/hoppip.gbapal");

// Arrows

static const u32 Gacha_Arrows_GFX[] = INCBIN_U32("graphics/gacha/arrows.4bpp.lz");

// Press "A"

static const u32 Gacha_Press_A_GFX[] = INCBIN_U32("graphics/gacha/pressA.4bpp.lz");

static const u16 sPokeball_Pal[] = INCBIN_U16("graphics/trade/pokeball.gbapal");
static const u8 sPokeball_Gfx[] = INCBIN_U8("graphics/trade/pokeball.4bpp");

const u16 gTrade_Tilemap[] = INCBIN_U16("graphics/trade/platform.bin");

#define GACHA_BG_BASE 1
#define GACHA_MENUS 2

static const struct BgTemplate sGachaBGtemplates[] = {
    {
       .bg = GACHA_BG_BASE,
       .charBaseIndex = 2,
       .mapBaseIndex = 31,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 3,
       .baseTile = 0
   },
   {
        .bg = GACHA_MENUS,
        .charBaseIndex = 0,
        .mapBaseIndex = 0x17,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

static const struct WindowTemplate sGachaWinTemplates[] = {
	{
        .bg = GACHA_MENUS,
        .tilemapLeft = 16,
        .tilemapTop = 9,
        .width = 14,
        .height = 2,
        .paletteNum = 0xF,
        .baseBlock = 0x194,		
	},
    DUMMY_WIN_TEMPLATE,
};

static const struct WindowTemplate sWinTemplates_EggHatch[] =
{
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 64
    },
    DUMMY_WIN_TEMPLATE
};

static const struct WindowTemplate sYesNoWinTemplate =
{
    .bg = 0,
    .tilemapLeft = 21,
    .tilemapTop = 9,
    .width = 5,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 424
};

#define BG_MIDDLE_GFX 1
#define BG_LEFT_GFX 2
#define BG_RIGHT_GFX 3
#define KNOB_GFX 4
#define DIGITAL_TEXT_GFX 5
#define LOTTERY_JPN_GFX 6
#define GFXTAG_CREDIT_DIGIT 7
#define GFXTAG_PLAYER_DIGIT 8
#define GFXTAG_MENU_1 9
#define GFXTAG_MENU_2 10
#define GFXTAG_BELOSSOM 11
#define GFXTAG_PHANPY 12
#define GFXTAG_TEDDIURSA 13
#define GFXTAG_ELEKID 14
#define GFXTAG_HOPPIP 15
#define GFXTAG_ARROWS 16
#define GFXTAG_PRESS_A 17

#define GFXTAG_POKEBALL        5557

#define BG_BASIC_PAL 1
#define BG_GREAT_PAL 2
#define BG_ULTRA_PAL 3
#define BG_MASTER_PAL 4
#define PALTAG_KNOB 5
#define DIGITAL_TEXT_PAL 6
#define LOTTERY_JPN_PAL 7
#define PALTAG_INTERFACE 8
#define PALTAG_INTERFACEPLAYER 9
#define PALTAG_MENU_BASIC 10
#define PALTAG_MENU_GREAT 11
#define PALTAG_MENU_ULTRA 12
#define PALTAG_MENU_MASTER 13

#define PALTAG_BELOSSOM 14
#define PALTAG_PHANPY 15
#define PALTAG_TEDDIURSA 16
#define PALTAG_ELEKID 17
#define PALTAG_HOPPIP 18
#define PALTAG_ARROWS 19
#define PALTAG_PRESS_A 20

#define PALTAG_POKEBALL  5558

static const struct SpritePalette sSpritePalettes[] =
{
    { .data = Gacha_BG_Basic_Pal,      .tag = BG_BASIC_PAL },
	{ .data = Gacha_BG_Great_Pal,      .tag = BG_GREAT_PAL },
	{ .data = Gacha_BG_Ultra_Pal,      .tag = BG_ULTRA_PAL },
	{ .data = Gacha_BG_Master_Pal,     .tag = BG_MASTER_PAL },
    {}
};

static const struct SpritePalette sSpritePalettes2[] =
{
	{ .data = BelossomPAL,			   .tag = PALTAG_BELOSSOM },
	{ .data = PhanpyPal,			   .tag = PALTAG_PHANPY },
	{ .data = TeddiursaPAL,			   .tag = PALTAG_TEDDIURSA },
	{ .data = ElekidPAL,			   .tag = PALTAG_ELEKID },
	{ .data = HoppipPAL,  			   .tag = PALTAG_HOPPIP },
	{ .data = sCredit_Pal,      	   .tag = PALTAG_ARROWS },	
	{ .data = Gacha_press_a_Pal,	   .tag = PALTAG_PRESS_A },	
	{ .data = Gacha_Knob_Pal,		   .tag = PALTAG_KNOB },
	{ .data = Gacha_Digital_Text_Pal,  .tag = DIGITAL_TEXT_PAL },	
	{ .data = sCredit_Pal,      	   .tag = PALTAG_INTERFACE },
	{ .data = sPlayer_Pal,			   .tag = PALTAG_INTERFACEPLAYER },	
	{ .data = Gacha_Lottery_Pal,	   .tag = LOTTERY_JPN_PAL },
	{ .data = Gacha_Menu_Basic_Pal,    .tag = PALTAG_MENU_BASIC },
	{ .data = Gacha_Menu_Great_Pal,    .tag = PALTAG_MENU_GREAT },
	{ .data = Gacha_Menu_Ultra_Pal,    .tag = PALTAG_MENU_ULTRA },
	{ .data = Gacha_Menu_Master_Pal,   .tag = PALTAG_MENU_MASTER },
	{}
};

static const struct SpritePalette sBall[] =
{
	{ .data = sPokeball_Pal,			   .tag = PALTAG_POKEBALL },
	{}
};

static const struct CompressedSpriteSheet sSpriteSheet_Press_A =
{
    .data = Gacha_Press_A_GFX,
    .size = 0xC00,
    .tag = GFXTAG_PRESS_A,
};

static const struct CompressedSpriteSheet sSpriteSheet_Arrows =
{
    .data = Gacha_Arrows_GFX,
    .size = 0x200,
    .tag = GFXTAG_ARROWS,
};

static const struct CompressedSpriteSheet sSpriteSheet_Hoppip =
{
    .data = HoppipGFX,
    .size = 0x800,
    .tag = GFXTAG_HOPPIP,
};

static const struct CompressedSpriteSheet sSpriteSheet_Elekid =
{
    .data = ElekidGFX,
    .size = 0x800,
    .tag = GFXTAG_ELEKID,
};

static const struct CompressedSpriteSheet sSpriteSheet_Teddiursa =
{
    .data = TeddiursaGFX,
    .size = 0x800,
    .tag = GFXTAG_TEDDIURSA,
};

static const struct CompressedSpriteSheet sSpriteSheet_Phanpy =
{
    .data = PhanpyGFX,
    .size = 0x800,
    .tag = GFXTAG_PHANPY,
};

static const struct CompressedSpriteSheet sSpriteSheet_Belossom =
{
    .data = BelossomGFX,
    .size = 0x800,
    .tag = GFXTAG_BELOSSOM,
};

static const struct CompressedSpriteSheet sSpriteSheet_Menu_1 =
{
    .data = Gacha_Menu_1,
    .size = 0x800,
    .tag = GFXTAG_MENU_1,
};

static const struct CompressedSpriteSheet sSpriteSheet_Menu_2 =
{
    .data = Gacha_Menu_2,
    .size = 0x1000,
    .tag = GFXTAG_MENU_2,
};

static const struct CompressedSpriteSheet sSpriteSheets_Interface[] =
{
    {
        .data = gCredits_Gfx,
        .size = 0x280,
        .tag = GFXTAG_CREDIT_DIGIT,
    },
    {}
};

static const struct CompressedSpriteSheet sSpriteSheets_PlayerInterface[] =
{
    {
        .data = gPlayer_Gfx,
        .size = 0x280,
        .tag = GFXTAG_PLAYER_DIGIT
    },
    {}
};

static const struct CompressedSpriteSheet sSpriteSheet_Lottery_JPN =
{
    .data = Gacha_Lottery_JPN,
    .size = 0x800,
    .tag = LOTTERY_JPN_GFX,
};

static const struct CompressedSpriteSheet sSpriteSheet_Digital_Text =
{
    .data = Gacha_Digital_Text,
    .size = 0x1000,
    .tag = DIGITAL_TEXT_GFX,
};

static const struct CompressedSpriteSheet sSpriteSheet_Knob =
{
    .data = Gacha_Knob,
    .size = 0x800,
    .tag = KNOB_GFX,
};

static const struct OamData sOamData_Press_A =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Arrows =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(8x32),
    .size = SPRITE_SIZE(8x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Hoppip =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Elekid =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Teddiursa =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Phanpy =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Belossom =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Menu =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Menu_2 =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Lottery_JPN =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Digital_Text =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOamData_Knob =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
	.tileNum = 0,
    .priority = 0,
};

static const struct OamData sOam_CreditDigit =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(8x16),
    .size = SPRITE_SIZE(8x16),
    .priority = 2,
};

static const struct OamData sOamData_Pokeball =
{
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16)
};

static const union AnimCmd sAnim_Pokeball_SpinOnce[] =
{
    ANIMCMD_FRAME( 0, 3),
    ANIMCMD_FRAME( 4, 3),
    ANIMCMD_FRAME( 8, 3),
    ANIMCMD_FRAME(12, 3),
    ANIMCMD_FRAME(16, 3),
    ANIMCMD_FRAME(20, 3),
    ANIMCMD_FRAME(24, 3),
    ANIMCMD_FRAME(28, 3),
    ANIMCMD_FRAME(32, 3),
    ANIMCMD_FRAME(36, 3),
    ANIMCMD_FRAME(40, 3),
    ANIMCMD_FRAME(44, 3),
    ANIMCMD_LOOP(1),
    ANIMCMD_FRAME( 0, 3),
    ANIMCMD_END
};

static const union AnimCmd sAnim_Pokeball_SpinTwice[] =
{
    ANIMCMD_FRAME( 0, 3),
    ANIMCMD_FRAME( 4, 3),
    ANIMCMD_FRAME( 8, 3),
    ANIMCMD_FRAME(12, 3),
    ANIMCMD_FRAME(16, 3),
    ANIMCMD_FRAME(20, 3),
    ANIMCMD_FRAME(24, 3),
    ANIMCMD_FRAME(28, 3),
    ANIMCMD_FRAME(32, 3),
    ANIMCMD_FRAME(36, 3),
    ANIMCMD_FRAME(40, 3),
    ANIMCMD_FRAME(44, 3),
    ANIMCMD_LOOP(2),
    ANIMCMD_FRAME( 0, 3),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_Pokeball[] =
{
    sAnim_Pokeball_SpinOnce,
    sAnim_Pokeball_SpinTwice
};

static const union AffineAnimCmd sAffineAnim_Pokeball_Normal[] =
{
    AFFINEANIMCMD_FRAME(0, 0, 0, 1),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_Pokeball_Squish[] =
{
    AFFINEANIMCMD_FRAME(-8, 0, 0, 20),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_Pokeball_Unsquish[] =
{
    AFFINEANIMCMD_FRAME(0x60, 0x100, 0,  0),
    AFFINEANIMCMD_FRAME(   0,     0, 0,  5),
    AFFINEANIMCMD_FRAME(   8,     0, 0, 20),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd *const sAffineAnims_Pokeball[] =
{
    sAffineAnim_Pokeball_Normal,
    sAffineAnim_Pokeball_Squish,
    sAffineAnim_Pokeball_Unsquish
};

static const struct SpriteSheet sPokeBallSpriteSheet =
{
    .data = sPokeball_Gfx,
    .size = sizeof(sPokeball_Gfx),
    .tag = GFXTAG_POKEBALL
};

static const struct SpritePalette sPokeBallSpritePalette =
{
    .data = sPokeball_Pal,
    .tag = PALTAG_POKEBALL
};

static const struct SpriteTemplate sSpriteTemplate_Pokeball =
{
    .tileTag = GFXTAG_POKEBALL,
    .paletteTag = PALTAG_POKEBALL,
    .oam = &sOamData_Pokeball,
    .anims = sAnims_Pokeball,
    .images = NULL,
    .affineAnims = sAffineAnims_Pokeball,
    .callback = SpriteCB_BouncingPokeball
};

static const union AnimCmd sPressAAnimCmd_1[] = 
{
	ANIMCMD_FRAME(32, 10),
	ANIMCMD_FRAME(64, 10),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd sPressAAnimCmd_0[] = 
{
	ANIMCMD_FRAME(0, 10),
	ANIMCMD_FRAME(0, 10),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sPressAAnimCmds[] = {
    sPressAAnimCmd_0, // Gray
	sPressAAnimCmd_1, // Highlight
};

static const struct SpriteTemplate sSpriteTemplate_Press_A =
{
    .tileTag = GFXTAG_PRESS_A,
    .paletteTag = PALTAG_PRESS_A,
    .oam = &sOamData_Press_A,
    .anims = sPressAAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const union AnimCmd sArrowAnimCmd_1[] = 
{
	ANIMCMD_FRAME(8, 20),
	ANIMCMD_FRAME(12, 20),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd sArrowAnimCmd_0[] = 
{
	ANIMCMD_FRAME(0, 20),
	ANIMCMD_FRAME(4, 20),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sArrowsAnimCmds[] = {
    sArrowAnimCmd_0, // Up and Down
	sArrowAnimCmd_1, // Up
};

static const struct SpriteTemplate sSpriteTemplate_Arrows =
{
    .tileTag = GFXTAG_ARROWS,
    .paletteTag = PALTAG_ARROWS,
    .oam = &sOamData_Arrows,
    .anims = sArrowsAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const union AnimCmd sMenu2AnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 10),
    ANIMCMD_FRAME(64, 10),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sMenu2AnimCmds[] = {
    sMenu2AnimCmd_0,  // Looping animation
};

static const union AnimCmd sHoppipAnimCmd_0[] = 
{
	ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(32, 15),
    ANIMCMD_FRAME(48, 15),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sHoppipAnimCmds[] = {
    sHoppipAnimCmd_0,  // Looping animation
};

static const struct SpriteTemplate sSpriteTemplate_Hoppip =
{
    .tileTag = GFXTAG_HOPPIP,
    .paletteTag = PALTAG_HOPPIP,
    .oam = &sOamData_Hoppip,
    .anims = sHoppipAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const union AnimCmd sElekidAnimCmd_0[] = 
{
	ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(32, 15),
	ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(32, 15),
	//ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(48, 15),
	ANIMCMD_FRAME(32, 15),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sElekidAnimCmds[] = {
    sElekidAnimCmd_0,  // Looping animation
};

static const struct SpriteTemplate sSpriteTemplate_Elekid =
{
    .tileTag = GFXTAG_ELEKID,
    .paletteTag = PALTAG_ELEKID,
    .oam = &sOamData_Elekid,
    .anims = sElekidAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const union AnimCmd sTeddiursaAnimCmd_0[] = 
{
    ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(32, 15),
	ANIMCMD_FRAME(16, 15),
    ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(32, 15),
	ANIMCMD_FRAME(16, 15),
    ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(32, 15),
	ANIMCMD_FRAME(16, 15),
    ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(32, 15),
	ANIMCMD_FRAME(16, 15),
    ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(32, 15),
	ANIMCMD_FRAME(16, 15),
    ANIMCMD_FRAME(48, 30),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sTeddiursaAnimCmds[] = {
    sTeddiursaAnimCmd_0,  // Looping animation
};

static const struct SpriteTemplate sSpriteTemplate_Teddiursa =
{
    .tileTag = GFXTAG_TEDDIURSA,
    .paletteTag = PALTAG_TEDDIURSA,
    .oam = &sOamData_Teddiursa,
    .anims = sTeddiursaAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const union AnimCmd sPhanpyAnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(48, 15),
	ANIMCMD_FRAME(32, 15),
	ANIMCMD_FRAME(48, 15),
	ANIMCMD_FRAME(16, 15),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sPhanpyAnimCmds[] = {
    sPhanpyAnimCmd_0,  // Looping animation
};

static const struct SpriteTemplate sSpriteTemplate_Phanpy =
{
    .tileTag = GFXTAG_PHANPY,
    .paletteTag = PALTAG_PHANPY,
    .oam = &sOamData_Phanpy,
    .anims = sPhanpyAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const union AnimCmd sBelossomAnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(32, 15),
	ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(16, 15),
	ANIMCMD_FRAME(0, 15),
	ANIMCMD_FRAME(48, 30),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sBelossomAnimCmds[] = {
    sBelossomAnimCmd_0,  // Looping animation
};

static const struct SpriteTemplate sSpriteTemplate_Belossom =
{
    .tileTag = GFXTAG_BELOSSOM,
    .paletteTag = PALTAG_BELOSSOM,
    .oam = &sOamData_Belossom,
    .anims = sBelossomAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Menu_1_Master =
{
    .tileTag = GFXTAG_MENU_1,
    .paletteTag = PALTAG_MENU_MASTER,
    .oam = &sOamData_Menu,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Menu_2_Master =
{
    .tileTag = GFXTAG_MENU_2,
    .paletteTag = PALTAG_MENU_MASTER,
    .oam = &sOamData_Menu_2,
    .anims = sMenu2AnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Menu_1_Ultra =
{
    .tileTag = GFXTAG_MENU_1,
    .paletteTag = PALTAG_MENU_ULTRA,
    .oam = &sOamData_Menu,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Menu_2_Ultra =
{
    .tileTag = GFXTAG_MENU_2,
    .paletteTag = PALTAG_MENU_ULTRA,
    .oam = &sOamData_Menu_2,
    .anims = sMenu2AnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Menu_1_Great =
{
    .tileTag = GFXTAG_MENU_1,
    .paletteTag = PALTAG_MENU_GREAT,
    .oam = &sOamData_Menu,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Menu_2_Great =
{
    .tileTag = GFXTAG_MENU_2,
    .paletteTag = PALTAG_MENU_GREAT,
    .oam = &sOamData_Menu_2,
    .anims = sMenu2AnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Menu_1_Basic =
{
    .tileTag = GFXTAG_MENU_1,
    .paletteTag = PALTAG_MENU_BASIC,
    .oam = &sOamData_Menu,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Menu_2_Basic =
{
    .tileTag = GFXTAG_MENU_2,
    .paletteTag = PALTAG_MENU_BASIC,
    .oam = &sOamData_Menu_2,
    .anims = sMenu2AnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_CreditDigit =
{
    .tileTag = GFXTAG_CREDIT_DIGIT,
    .paletteTag = PALTAG_INTERFACE,
    .oam = &sOam_CreditDigit,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const struct SpriteTemplate sSpriteTemplate_PlayerDigit =
{
    .tileTag = GFXTAG_PLAYER_DIGIT,
    .paletteTag = PALTAG_INTERFACEPLAYER,
    .oam = &sOam_CreditDigit,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const union AnimCmd sDigitalTextAnimCmd_0[] = 
{
    ANIMCMD_FRAME(0, 30),
    ANIMCMD_FRAME(32, 30),
    ANIMCMD_FRAME(64, 30),
    ANIMCMD_FRAME(96, 30),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sDigitalTextAnimCmds[] = {
    sDigitalTextAnimCmd_0,  // Looping animation
};

static const struct SpriteTemplate sSpriteTemplate_Digital_Text =
{
    .tileTag = DIGITAL_TEXT_GFX,
    .paletteTag = DIGITAL_TEXT_PAL,
    .oam = &sOamData_Digital_Text,
    .anims = sDigitalTextAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Lottery_JPN =
{
    .tileTag = LOTTERY_JPN_GFX,
    .paletteTag = LOTTERY_JPN_PAL,
    .oam = &sOamData_Lottery_JPN,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const union AnimCmd sKnobAnimCmd_1[] = 
{
	ANIMCMD_FRAME(0, 5),
	ANIMCMD_FRAME(16, 5),
	ANIMCMD_FRAME(32, 20),
	ANIMCMD_FRAME(16, 5),
	ANIMCMD_FRAME(0, 5),
    ANIMCMD_END
};

static const union AnimCmd sKnobAnimCmd_0[] = 
{
	ANIMCMD_FRAME(0, 20),
    ANIMCMD_END
};

static const union AnimCmd *const sKnobAnimCmds[] = {
    sKnobAnimCmd_0, // Still
	sKnobAnimCmd_1, // Rotate
};

static const struct SpriteTemplate sSpriteTemplate_Knob =
{
    .tileTag = KNOB_GFX,
    .paletteTag = PALTAG_KNOB,
    .oam = &sOamData_Knob,
    .anims = sKnobAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

void StartGacha(void)
{
	u8 taskId = 0;
    sGacha = AllocZeroed(sizeof(struct Gacha));
    taskId = CreateTask(FadeToGachaScreen, 0);
}

static void SpriteCB_BouncingPokeball(struct Sprite *sprite)
{
    sprite->y += sprite->data[0] / 10;
    sprite->data[5] += sprite->data[1];
    sprite->x = sprite->data[5] / 10;
    if (sprite->y > 0x4c)
    {
        sprite->y = 0x4c;
        sprite->data[0] = -(sprite->data[0] * sprite->data[2]) / 100;
        sprite->data[3] ++;
    }
    if (sprite->x == 0x78)
        sprite->data[1] = 0;
    sprite->data[0] += sprite->data[4];
    if (sprite->data[3] == 4)
    {
        sprite->data[7] = 1;
        sprite->callback = SpriteCallbackDummy;
    }
}

static void SpriteCB_BouncingPokeballArrive(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        if ((sprite->y += 4) > sprite->data[3])
        {
            sprite->data[2] ++;
            sprite->data[0] = 0x16;
            PlaySE(SE_BALL_BOUNCE_1);
        }
    }
    else
    {
        if (sprite->data[0] == 0x42)
            PlaySE(SE_BALL_BOUNCE_2);
        if (sprite->data[0] == 0x5c)
            PlaySE(SE_BALL_BOUNCE_3);
        if (sprite->data[0] == 0x6b)
            PlaySE(SE_BALL_BOUNCE_4);
        sprite->y2 += sTradeBallVerticalVelocityTable[sprite->data[0]];
        if (++sprite->data[0] == 0x6c)
            sprite->callback = SpriteCallbackDummy;
    }
}

static void FadeToGachaScreen(u8 taskId)
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
            SetMainCallback2(InitGachaScreen);
            DestroyTask(taskId);
        }
        break;
    }
}

static void BGSetup(void)
{
	u16 size = 0x1480;
	
	if (sGacha->GachaId == 1) // Basic
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Main, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Main_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Basic_Pal, 0, sizeof(Gacha_BG_Basic_Pal));
	}
	else if (sGacha->GachaId == 2) // Great
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Main, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Main_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Great_Pal, 0, sizeof(Gacha_BG_Great_Pal));		
	}
	else if (sGacha->GachaId == 3) // Ultra
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Main, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Main_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Ultra_Pal, 0, sizeof(Gacha_BG_Ultra_Pal));		
	}
	else // Master
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Main, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Main_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Master_Pal, 0, sizeof(Gacha_BG_Master_Pal));		
	}
}

static void BGRed(void)
{
	u16 size = 0x1480;

		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Red, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Red_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Red_Pal, 0, sizeof(Gacha_BG_Red_Pal));
}

static void Shake1(void)
{
	u16 size = 0x1480;
	
	if (sGacha->GachaId == 1) // Basic
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Left, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Left_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Basic_Pal, 0, sizeof(Gacha_BG_Basic_Pal));
	}
	else if (sGacha->GachaId == 2) // Great
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Left, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Left_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Great_Pal, 0, sizeof(Gacha_BG_Great_Pal));		
	}
	else if (sGacha->GachaId == 3) // Ultra
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Left, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Left_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Ultra_Pal, 0, sizeof(Gacha_BG_Ultra_Pal));		
	}
	else // Master
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Left, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Left_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Master_Pal, 0, sizeof(Gacha_BG_Master_Pal));		
	}
}

static void Shake2(void)
{
	u16 size = 0x1480;
	
	if (sGacha->GachaId == 1) // Basic
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Right, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Right_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Basic_Pal, 0, sizeof(Gacha_BG_Basic_Pal));
	}
	else if (sGacha->GachaId == 2) // Great
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Right, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Right_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Great_Pal, 0, sizeof(Gacha_BG_Great_Pal));		
	}
	else if (sGacha->GachaId == 3) // Ultra
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Right, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Right_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Ultra_Pal, 0, sizeof(Gacha_BG_Ultra_Pal));		
	}
	else // Master
	{
		InitBgsFromTemplates(0, sGachaBGtemplates, ARRAY_COUNT(sGachaBGtemplates));
		SetBgTilemapBuffer(GACHA_BG_BASE, AllocZeroed(BG_SCREEN_SIZE));
		DecompressAndLoadBgGfxUsingHeap(GACHA_BG_BASE, Gacha_BG_Right, size, 0, 0);
		CopyToBgTilemapBuffer(GACHA_BG_BASE, Gacha_BG_Right_Tilemap, 0, 0);
		ResetPaletteFade();
		LoadPalette(Gacha_BG_Master_Pal, 0, sizeof(Gacha_BG_Master_Pal));		
	}
}

static void GachaVBlankCallback(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void GachaMainCallback(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
}

static void SetCreditDigits(u16 num)
{
    u8 i;
    u16 d = 1000;

    for (i = 0; i < 4; i++)
    {
        u8 digit = num / d;

        gSprites[sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].invisible = FALSE;

        gSprites[sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].oam.tileNum =
            gSprites[sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].sheetTileStart + (digit * 2);

        gSprites[sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].oam.priority = 0;

        num = num % d;
        d = d / 10;
    }

    for (i = 0; i < 4; i++) {
        if (gSprites[sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].invisible == FALSE) {

        } else {
            gSprites[sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS]].invisible = FALSE;
        }
    }

    BuildOamBuffer();
}

static void SetPlayerDigits(u16 num)
{
    u8 i;
    u16 d = 1000;  // Start with the thousands place

    for (i = 0; i < 4; i++)  // Always show 4 digits
    {
        u8 digit = num / d;

        // Show the digit (all digits are visible)
        gSprites[sGacha->PlayerSpriteIds[i + SPR_PLAYER_DIGITS]].invisible = FALSE;

        // If it's a smaller number, show 0 for the higher place values
        if (i == 0 && num < 1000) {
            digit = 0;  // Force 0 for the thousands place if the number is less than 1000
        }

        // Set the tileNum based on the current digit
        gSprites[sGacha->PlayerSpriteIds[i + SPR_PLAYER_DIGITS]].oam.tileNum =
            gSprites[sGacha->PlayerSpriteIds[i + SPR_PLAYER_DIGITS]].sheetTileStart + (digit * 2);

        gSprites[sGacha->PlayerSpriteIds[i + SPR_PLAYER_DIGITS]].oam.priority = 0;

        // Reduce num for the next digit
        num = num % d;
        d = d / 10;
    }

    BuildOamBuffer();
}

static void CreateCreditSprites(void)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sSpriteSheets_Interface) - 1; i++)  
    {
        struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheets_Interface[i].data, gDecompressionBuffer);
        s.data = gDecompressionBuffer;
        s.size = sSpriteSheets_Interface[i].size;
        s.tag = sSpriteSheets_Interface[i].tag;
        LoadSpriteSheet(&s);
    }

    for (i = 0; i < MAX_COIN_DIGITS; i++)
    {
		if (i == 0){
        sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS] = CreateSprite(&sSpriteTemplate_CreditDigit, 207, 140, 2);
		gSprites[sGacha->PlayerSpriteIds[i + SPR_CREDIT_DIGITS]].oam.priority = 0;
		}
		if (i == 1){
        sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS] = CreateSprite(&sSpriteTemplate_CreditDigit, 8 + 207, 140, 2);
		gSprites[sGacha->PlayerSpriteIds[i + SPR_CREDIT_DIGITS]].oam.priority = 0;
		}
		if (i == 2){
        sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS] = CreateSprite(&sSpriteTemplate_CreditDigit, 16 + 207, 140, 2);
		gSprites[sGacha->PlayerSpriteIds[i + SPR_CREDIT_DIGITS]].oam.priority = 0;
		}
		if (i == 3){
        sGacha->CreditSpriteIds[i + SPR_CREDIT_DIGITS] = CreateSprite(&sSpriteTemplate_CreditDigit, 24 + 207, 140, 2);
		gSprites[sGacha->PlayerSpriteIds[i + SPR_CREDIT_DIGITS]].oam.priority = 0;
		}
    }
}

static void CreatePlayerSprites(void)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sSpriteSheets_PlayerInterface) - 1; i++)  
    {
        struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheets_PlayerInterface[i].data, gDecompressionBuffer);
        s.data = gDecompressionBuffer;
        s.size = sSpriteSheets_PlayerInterface[i].size;
        s.tag = sSpriteSheets_PlayerInterface[i].tag;
        LoadSpriteSheet(&s);
    }

    for (i = 0; i < 4; i++)
    {
        sGacha->PlayerSpriteIds[i + SPR_PLAYER_DIGITS] = CreateSprite(&sSpriteTemplate_PlayerDigit, i * 8 + 207, 118, 2);
		gSprites[sGacha->PlayerSpriteIds[i + SPR_PLAYER_DIGITS]].oam.priority = 0;
    }
}

static void CreateCTA(void)
{
		struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Press_A.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Press_A.size;
		s.tag = GFXTAG_PRESS_A;
		LoadSpriteSheet(&s);
	
	sGacha->CTAspriteId = CreateSprite(&sSpriteTemplate_Press_A, 152, 116, 0);
	gSprites[sGacha->CTAspriteId].animNum = 0; // Off
}

static void CreateArrows(void)
{
		struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Arrows.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Arrows.size;
		s.tag = GFXTAG_ARROWS;
		LoadSpriteSheet(&s);
	
	sGacha->ArrowsSpriteId = CreateSprite(&sSpriteTemplate_Arrows, 207 + 24, 120, 0);
	gSprites[sGacha->ArrowsSpriteId].animNum = 1; // Only Up
}

static void CreateLotteryJPN(void)
{
		struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Lottery_JPN.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Lottery_JPN.size;
		s.tag = LOTTERY_JPN_GFX;
		LoadSpriteSheet(&s);
	
	sGacha->LotteryJPNspriteId = CreateSprite(&sSpriteTemplate_Lottery_JPN, 176, 32, 0);
}

static void CreateHoppip(void)
{
	s16 x = 142;
	s16 y = 56;
	s16 x2 = x + 34;
	s16 x3 = x + 68;
	
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Hoppip.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Hoppip.size;
		s.tag = GFXTAG_HOPPIP;
		LoadSpriteSheet(&s);
	
	sGacha->PokemonOneSpriteId = CreateSprite(&sSpriteTemplate_Hoppip, x, y, 0);
	sGacha->PokemonTwoSpriteId = CreateSprite(&sSpriteTemplate_Hoppip, x2, y, 0);	
	sGacha->PokemonThreeSpriteId = CreateSprite(&sSpriteTemplate_Hoppip, x3, y, 0);	
}

static void CreateElekid(void)
{
	s16 x = 142;
	s16 y = 56 + 2;
	s16 x2 = x + 34;
	s16 x3 = x + 68;
	
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Elekid.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Elekid.size;
		s.tag = GFXTAG_ELEKID;
		LoadSpriteSheet(&s);
	
	sGacha->PokemonOneSpriteId = CreateSprite(&sSpriteTemplate_Elekid, x, y, 0);
	sGacha->PokemonTwoSpriteId = CreateSprite(&sSpriteTemplate_Elekid, x2, y, 0);
	sGacha->PokemonThreeSpriteId = CreateSprite(&sSpriteTemplate_Elekid, x3, y, 0);	
}

static void CreateTeddiursa(void)
{
	s16 x = 142;
	s16 y = 56;
	s16 x2 = x + 34;
	s16 x3 = x + 68;
	
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Teddiursa.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Teddiursa.size;
		s.tag = GFXTAG_TEDDIURSA;
		LoadSpriteSheet(&s);
	
	sGacha->PokemonOneSpriteId = CreateSprite(&sSpriteTemplate_Teddiursa, x, y, 0);
	sGacha->PokemonTwoSpriteId = CreateSprite(&sSpriteTemplate_Teddiursa, x2, y, 0);	
	sGacha->PokemonThreeSpriteId = CreateSprite(&sSpriteTemplate_Teddiursa, x3, y, 0);
}

static void CreatePhanpy(void)
{
	s16 x = 142;
	s16 y = 56;
	s16 x2 = x + 34;
	s16 x3 = x + 68;
	
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Phanpy.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Phanpy.size;
		s.tag = GFXTAG_PHANPY;
		LoadSpriteSheet(&s);
	
	sGacha->PokemonOneSpriteId = CreateSprite(&sSpriteTemplate_Phanpy, x, y, 0);
	sGacha->PokemonTwoSpriteId = CreateSprite(&sSpriteTemplate_Phanpy, x2, y, 0);
	sGacha->PokemonThreeSpriteId = CreateSprite(&sSpriteTemplate_Phanpy, x3, y, 0);
}

static void CreateBelossom(void)
{
	s16 x = 142;
	s16 y = 56;
	s16 x2 = x + 34;
	s16 x3 = x + 68;
	
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Belossom.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Belossom.size;
		s.tag = GFXTAG_BELOSSOM;
		LoadSpriteSheet(&s);
	
	sGacha->PokemonOneSpriteId = CreateSprite(&sSpriteTemplate_Belossom, x, y, 0);
	sGacha->PokemonTwoSpriteId = CreateSprite(&sSpriteTemplate_Belossom, x2, y, 0);	
	sGacha->PokemonThreeSpriteId = CreateSprite(&sSpriteTemplate_Belossom, x3, y, 0);

}

static void CreateDigitalText(void)
{
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Digital_Text.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Digital_Text.size;
		s.tag = DIGITAL_TEXT_GFX;
		LoadSpriteSheet(&s);
	
	sGacha->DigitalTextSpriteId = CreateSprite(&sSpriteTemplate_Digital_Text, 64, 25, 0);
}

static void CreateCreditMenu(void)
{
	s16 x = 144;
	s16 y = 128;
	s16 x2 = x + 64;
	u8 priority = 1;
	
	if (sGacha->GachaId == 1) // Basic
	{
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Menu_1.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Menu_1.size;
		s.tag = GFXTAG_MENU_1;
		LoadSpriteSheet(&s);
	
	sGacha->CreditMenu1Id = CreateSprite(&sSpriteTemplate_Menu_1_Basic, x, y, priority);
	gSprites[sGacha->CreditMenu1Id].oam.priority = 1;
	}
	else if (sGacha->GachaId == 2) // Great
	{
	struct SpriteSheet s;	
        LZ77UnCompWram(sSpriteSheet_Menu_1.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Menu_1.size;
		s.tag = GFXTAG_MENU_1;
		LoadSpriteSheet(&s);
	
	sGacha->CreditMenu1Id = CreateSprite(&sSpriteTemplate_Menu_1_Great, x, y, priority);
	gSprites[sGacha->CreditMenu1Id].oam.priority = 1;
	}
	else if (sGacha->GachaId == 3) // Ultra
	{
	struct SpriteSheet s;	
        LZ77UnCompWram(sSpriteSheet_Menu_1.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Menu_1.size;
		s.tag = GFXTAG_MENU_1;
		LoadSpriteSheet(&s);
	
	sGacha->CreditMenu1Id = CreateSprite(&sSpriteTemplate_Menu_1_Ultra, x, y, priority);
	gSprites[sGacha->CreditMenu1Id].oam.priority = 1;
	}
	else // Master
	{
	struct SpriteSheet s;	
        LZ77UnCompWram(sSpriteSheet_Menu_1.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Menu_1.size;
		s.tag = GFXTAG_MENU_1;
		LoadSpriteSheet(&s);
	
	sGacha->CreditMenu1Id = CreateSprite(&sSpriteTemplate_Menu_1_Master, x, y, priority);
	gSprites[sGacha->CreditMenu1Id].oam.priority = 1;
	}
}

static void CreatePlayerMenu(void)
{
	s16 x = 144;
	s16 y = 128;
	s16 x2 = x + 64;
	u8 priority = 1;
	
	if (sGacha->GachaId == 1) // Basic
	{
	struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Menu_2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Menu_2.size;
		s.tag = GFXTAG_MENU_2;
		LoadSpriteSheet(&s);
	
	sGacha->CreditMenu2Id = CreateSprite(&sSpriteTemplate_Menu_2_Basic, x2, y, priority);
	gSprites[sGacha->CreditMenu2Id].oam.priority = 1;
	}
	else if (sGacha->GachaId == 2) // Great
	{
	struct SpriteSheet s;	
        LZ77UnCompWram(sSpriteSheet_Menu_2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Menu_2.size;
		s.tag = GFXTAG_MENU_2;
		LoadSpriteSheet(&s);
	
	sGacha->CreditMenu2Id = CreateSprite(&sSpriteTemplate_Menu_2_Great, x2, y, priority);
	gSprites[sGacha->CreditMenu2Id].oam.priority = 1;
	}
	else if (sGacha->GachaId == 3) // Ultra
	{
	struct SpriteSheet s;	
        LZ77UnCompWram(sSpriteSheet_Menu_2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Menu_2.size;
		s.tag = GFXTAG_MENU_2;
		LoadSpriteSheet(&s);
	
	sGacha->CreditMenu2Id = CreateSprite(&sSpriteTemplate_Menu_2_Ultra, x2, y, priority);
	gSprites[sGacha->CreditMenu2Id].oam.priority = 1;
	}
	else // Master
	{
	struct SpriteSheet s;	
        LZ77UnCompWram(sSpriteSheet_Menu_2.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Menu_2.size;
		s.tag = GFXTAG_MENU_2;
		LoadSpriteSheet(&s);
	
	sGacha->CreditMenu2Id = CreateSprite(&sSpriteTemplate_Menu_2_Master, x2, y, priority);
	gSprites[sGacha->CreditMenu2Id].oam.priority = 1;
	}
}

static void CreateKnob(void)
{
		struct SpriteSheet s;
        LZ77UnCompWram(sSpriteSheet_Knob.data, gDecompressionBuffer);
		s.data = gDecompressionBuffer;
		s.size = sSpriteSheet_Knob.size;
		s.tag = KNOB_GFX;
		LoadSpriteSheet(&s);
	
	sGacha->KnobSpriteId = CreateSprite(&sSpriteTemplate_Knob, 76, 128, 0);
	gSprites[sGacha->KnobSpriteId].animNum = 0; // No Rotation
}

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesBasicA;

static const SpeciesBasicA SpeciesBasicCommon[] = {
    {0, SPECIES_SUNKERN},
    {1, SPECIES_AZURILL},
    {2, SPECIES_CATERPIE},
    {3, SPECIES_WURMPLE},
    {4, SPECIES_IGGLYBUFF},
    {5, SPECIES_WOOPER},
    {6, SPECIES_MAGIKARP},
    {7, SPECIES_SENTRET},
    {8, SPECIES_CLEFFA},
    {9, SPECIES_POOCHYENA},
    {10, SPECIES_LOTAD},
    {11, SPECIES_SEEDOT},
    {12, SPECIES_ZIGZAGOON},
    {13, SPECIES_WHISMUR},
    {14, SPECIES_ZUBAT},
    {15, SPECIES_SPINARAK},
    {16, SPECIES_HOPPIP},
    {17, SPECIES_PIDGEY},
    {18, SPECIES_RATTATA},
    {19, SPECIES_SPEAROW},
    {20, SPECIES_HOOTHOOT},
    {21, SPECIES_LEDYBA},
    {22, SPECIES_SURSKIT},
    {23, SPECIES_TAILLOW},
    {24, SPECIES_WINGULL},
    {25, SPECIES_NIDORAN_M},
    {26, SPECIES_NIDORAN_F},
    {27, SPECIES_PARAS},
    {28, SPECIES_SHROOMISH},
    {29, SPECIES_POLIWAG},
    {30, SPECIES_BELLSPROUT},
    {31, SPECIES_VENONAT},
    {32, SPECIES_SMOOCHUM},
    {33, SPECIES_ODDISH},
    {34, SPECIES_PSYDUCK},
    {35, SPECIES_GOLDEEN},
    {36, SPECIES_FEEBAS}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesBasicB;

static const SpeciesBasicB SpeciesBasicUncommon[] = {
    {0, SPECIES_METAPOD},
    {1, SPECIES_KAKUNA},
    {2, SPECIES_PICHU},
    {3, SPECIES_SILCOON},
    {4, SPECIES_CASCOON},
    {5, SPECIES_MAKUHITA},
    {6, SPECIES_MARILL},
    {7, SPECIES_SLUGMA},
    {8, SPECIES_SWINUB},
    {9, SPECIES_DIGLETT},
    {10, SPECIES_MAREEP},
    {11, SPECIES_MEDITITE},
    {12, SPECIES_EKANS},
    {13, SPECIES_BARBOACH},
    {14, SPECIES_HORSEA},
    {15, SPECIES_SANDSHREW},
    {16, SPECIES_GEODUDE},
    {17, SPECIES_GULPIN},
    {18, SPECIES_MANKEY},
    {19, SPECIES_MACHOP},
    {20, SPECIES_SHELLDER},
    {21, SPECIES_GRIMER},
    {22, SPECIES_VOLTORB},
    {23, SPECIES_PLUSLE},
    {24, SPECIES_MINUN},
    {25, SPECIES_NATU},
    {26, SPECIES_NINCADA},
    {27, SPECIES_SPOINK}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesBasicC;

static const SpeciesBasicC SpeciesBasicRare[] = {
    {0, SPECIES_RALTS},
    {1, SPECIES_TYROGUE},
    {2, SPECIES_SKITTY},
    {3, SPECIES_SLAKOTH},
    {4, SPECIES_MEOWTH},
    {5, SPECIES_PINECO},
    {6, SPECIES_TRAPINCH},
    {7, SPECIES_SPHEAL},
    {8, SPECIES_VULPIX},
    {9, SPECIES_SNUBBULL},
    {10, SPECIES_REMORAID},
    {11, SPECIES_CORPHISH},
    {12, SPECIES_ABRA},
    {13, SPECIES_ELEKID},
    {14, SPECIES_MAGBY},
    {15, SPECIES_CORSOLA}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesBasicD;

static const SpeciesBasicD SpeciesBasicUltraRare[] = {
    {0, SPECIES_TOGEPI},
    {1, SPECIES_CHARMANDER},
    {2, SPECIES_CYNDAQUIL},
    {3, SPECIES_TREECKO},
    {4, SPECIES_TORCHIC},
    {5, SPECIES_MUDKIP},
    {6, SPECIES_SQUIRTLE},
    {7, SPECIES_TOTODILE},
    {8, SPECIES_BULBASAUR},
    {9, SPECIES_CHIKORITA},
    {10, SPECIES_SHUCKLE}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesGreatA;

static const SpeciesGreatA SpeciesGreatCommon[] = {
    {0, SPECIES_JIGGLYPUFF},
    {1, SPECIES_CLEFAIRY},
    {2, SPECIES_KIRLIA},
    {3, SPECIES_ELECTRIKE},
    {4, SPECIES_SHUPPET},
    {5, SPECIES_DUSKULL},
    {6, SPECIES_SHELLDER},
    {7, SPECIES_MACHOP},
    {8, SPECIES_MANKEY},
    {9, SPECIES_GULPIN},
    {10, SPECIES_MAREEP},
    {11, SPECIES_PINECO},
    {12, SPECIES_REMORAID},
    {13, SPECIES_CARVANHA},
    {14, SPECIES_NUMEL},
    {15, SPECIES_CORPHISH},
    {16, SPECIES_SWABLU},
    {17, SPECIES_SLOWPOKE},
    {18, SPECIES_PIKACHU},
    {19, SPECIES_PSYDUCK},
    {20, SPECIES_GRIMER},
    {21, SPECIES_KRABBY},
    {22, SPECIES_EXEGGCUTE},
    {23, SPECIES_VOLTORB},
    {24, SPECIES_NATU},
    {25, SPECIES_PHANPY},
    {26, SPECIES_ARON},
    {27, SPECIES_SPOINK},
    {28, SPECIES_TENTACOOL},
    {29, SPECIES_KOFFING},
    {30, SPECIES_SKIPLOOM},
    {31, SPECIES_LOMBRE},
    {32, SPECIES_NUZLEAF},
    {33, SPECIES_RHYHORN},
    {34, SPECIES_CLAMPERL},
    {35, SPECIES_PIDGEOTTO},
    {36, SPECIES_ELEKID},
    {37, SPECIES_LOUDRED},
    {38, SPECIES_NIDORINA},
    {39, SPECIES_NIDORINO},
    {40, SPECIES_MAGBY},
    {41, SPECIES_POLIWHIRL},
    {42, SPECIES_ONIX},
    {43, SPECIES_GRAVELER},
    {44, SPECIES_GLOOM},
    {45, SPECIES_PLUSLE},
    {46, SPECIES_MINUN},
    {47, SPECIES_PONYTA},
    {48, SPECIES_FURRET},
    {49, SPECIES_LINOONE},
    {50, SPECIES_SUNFLORA},
    {51, SPECIES_CHIMECHO},
    {52, SPECIES_QUAGSIRE},
    {53, SPECIES_TAILLOW},
    {54, SPECIES_PELIPPER},
    {55, SPECIES_PERSIAN},
    {56, SPECIES_SEADRA},
    {57, SPECIES_NOCTOWL},
    {58, SPECIES_SANDSLASH},
    {59, SPECIES_VENOMOTH},
    {60, SPECIES_SEAKING},
    {61, SPECIES_GOLBAT},
    {62, SPECIES_TYROGUE},
    {63, SPECIES_TORKOAL},
    {64, SPECIES_ELECTRODE}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesGreatB;

static const SpeciesGreatB SpeciesGreatUncommon[] = {
    {0, SPECIES_FEEBAS},
    {1, SPECIES_BALTOY},
    {2, SPECIES_SNORUNT},
    {3, SPECIES_DODUO},
    {4, SPECIES_GASTLY},
    {5, SPECIES_ABRA},
    {6, SPECIES_CUBONE},
    {7, SPECIES_MAGNEMITE},
    {8, SPECIES_SEEL},
    {9, SPECIES_DROWZEE},
    {10, SPECIES_CHINCHOU},
    {11, SPECIES_TEDDIURSA},
    {12, SPECIES_HOUNDOUR},
    {13, SPECIES_CACNEA},
    {14, SPECIES_GROWLITHE},
    {15, SPECIES_SPINDA},
    {16, SPECIES_FLAAFFY},
    {17, SPECIES_CORSOLA},
    {18, SPECIES_DELCATTY},
    {19, SPECIES_DUSTOX},
    {20, SPECIES_WEEPINBELL},
    {21, SPECIES_LEDIAN},
    {22, SPECIES_ARIADOS},
    {23, SPECIES_BUTTERFREE},
    {24, SPECIES_BEEDRILL},
    {25, SPECIES_BEAUTIFLY},
    {26, SPECIES_VOLBEAT},
    {27, SPECIES_ILLUMISE},
    {28, SPECIES_ROSELIA},
    {29, SPECIES_WAILMER},
    {30, SPECIES_MACHOKE},
    {31, SPECIES_MURKROW},
    {32, SPECIES_MAGCARGO},
    {33, SPECIES_RATICATE},
    {34, SPECIES_MASQUERAIN},
    {35, SPECIES_MIGHTYENA},
    {36, SPECIES_CASTFORM},
    {37, SPECIES_GLIGAR},
    {38, SPECIES_QWILFISH},
    {39, SPECIES_TANGELA},
    {40, SPECIES_VIGOROTH},
    {41, SPECIES_FEAROW},
    {42, SPECIES_PILOSWINE},
    {43, SPECIES_PRIMEAPE},
    {44, SPECIES_BRELOOM},
    {45, SPECIES_TROPIUS},
    {46, SPECIES_STANTLER},
    {47, SPECIES_SWALOT},
    {48, SPECIES_XATU},
    {49, SPECIES_GRUMPIG},
    {50, SPECIES_HARIYAMA},
    {51, SPECIES_GOLDUCK}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesGreatC;

static const SpeciesGreatC SpeciesGreatRare[] = {
    {0, SPECIES_DRATINI},
    {1, SPECIES_LARVITAR},
    {2, SPECIES_BAGON},
    {3, SPECIES_TOGEPI},
    {4, SPECIES_CHARMANDER},
    {5, SPECIES_CYNDAQUIL},
    {6, SPECIES_TREECKO},
    {7, SPECIES_TORCHIC},
    {8, SPECIES_MUDKIP},
    {9, SPECIES_SQUIRTLE},
    {10, SPECIES_TOTODILE},
    {11, SPECIES_BULBASAUR},
    {12, SPECIES_CHIKORITA},
    {13, SPECIES_LUVDISC},
    {14, SPECIES_STARYU},
    {15, SPECIES_VIBRAVA},
    {16, SPECIES_FARFETCHD},
    {17, SPECIES_AIPOM},
    {18, SPECIES_NOSEPASS},
    {19, SPECIES_SABLEYE},
    {20, SPECIES_MAWILE},
    {21, SPECIES_YANMA},
    {22, SPECIES_KADABRA},
    {23, SPECIES_DUGTRIO},
    {24, SPECIES_HAUNTER},
    {25, SPECIES_SUDOWOODO},
    {26, SPECIES_KECLEON},
    {27, SPECIES_MEDICHAM},
    {28, SPECIES_SEALEO},
    {29, SPECIES_DUNSPARCE},
    {30, SPECIES_SNEASEL},
    {31, SPECIES_ZANGOOSE},
    {32, SPECIES_SEVIPER},
    {33, SPECIES_MANTINE},
    {34, SPECIES_SKARMORY},
    {35, SPECIES_OCTILLERY},
    {36, SPECIES_RELICANTH},
    {37, SPECIES_MILTANK},
    {38, SPECIES_SCYTHER},
    {39, SPECIES_PINSIR},
    {40, SPECIES_SHUCKLE}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesGreatD;

static const SpeciesGreatD SpeciesGreatUltraRare[] = {
    {0, SPECIES_WYNAUT},
    {1, SPECIES_DELIBIRD},
    {2, SPECIES_PORYGON},
    {3, SPECIES_IVYSAUR},
    {4, SPECIES_CHARMELEON},
    {5, SPECIES_WARTORTLE},
    {6, SPECIES_BAYLEEF},
    {7, SPECIES_QUILAVA},
    {8, SPECIES_CROCONAW},
    {9, SPECIES_GROVYLE},
    {10, SPECIES_COMBUSKEN},
    {11, SPECIES_MARSHTOMP},
    {12, SPECIES_PUPITAR},
    {13, SPECIES_DRAGONAIR},
    {14, SPECIES_SHELGON},
    {15, SPECIES_METANG},
    {16, SPECIES_MR_MIME},
    {17, SPECIES_HERACROSS}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesUltraA;

static const SpeciesUltraA SpeciesUltraCommon[] = {
    {0, SPECIES_KRABBY},
    {1, SPECIES_EXEGGCUTE},
    {2, SPECIES_MAGNEMITE},
    {3, SPECIES_TEDDIURSA},
    {4, SPECIES_ARIADOS},
    {5, SPECIES_PARASECT},
    {6, SPECIES_DUGTRIO},
    {7, SPECIES_SUDOWOODO},
    {8, SPECIES_MAGCARGO},
    {9, SPECIES_MEDICHAM},
    {10, SPECIES_SEALEO},
    {11, SPECIES_MASQUERAIN},
    {12, SPECIES_MIGHTYENA},
    {13, SPECIES_LINOONE},
    {14, SPECIES_CASTFORM},
    {15, SPECIES_SUNFLORA},
    {16, SPECIES_CHIMECHO},
    {17, SPECIES_SWELLOW},
    {18, SPECIES_PELIPPER},
    {19, SPECIES_LAIRON},
    {20, SPECIES_WIGGLYTUFF},
    {21, SPECIES_ARBOK},
    {22, SPECIES_KECLEON},
    {23, SPECIES_FEAROW},
    {24, SPECIES_SANDSLASH},
    {25, SPECIES_SEAKING},
    {26, SPECIES_NINJASK},
    {27, SPECIES_ZANGOOSE},
    {28, SPECIES_SEVIPER},
    {29, SPECIES_DODRIO},
    {30, SPECIES_LANTURN},
    {31, SPECIES_JUMPLUFF},
    {32, SPECIES_BRELOOM},
    {33, SPECIES_SHARPEDO},
    {34, SPECIES_CAMERUPT},
    {35, SPECIES_SWALOT},
    {36, SPECIES_CRAWDAUNT},
    {37, SPECIES_XATU},
    {38, SPECIES_TORKOAL},
    {39, SPECIES_GRUMPIG},
    {40, SPECIES_HARIYAMA},
    {41, SPECIES_KINGLER},
    {42, SPECIES_PIDGEOT},
    {43, SPECIES_CLEFABLE},
    {44, SPECIES_HYPNO},
    {45, SPECIES_RAICHU},
    {46, SPECIES_RHYDON},
    {47, SPECIES_VILEPLUME},
    {48, SPECIES_VICTREEBEL},
    {49, SPECIES_BELLOSSOM},
    {50, SPECIES_MILTANK},
    {51, SPECIES_GOLEM},
    {52, SPECIES_GOLDUCK},
    {53, SPECIES_RAPIDASH},
    {54, SPECIES_WAILORD},
    {55, SPECIES_NIDOQUEEN},
    {56, SPECIES_NIDOKING},
    {57, SPECIES_NINETALES},
    {58, SPECIES_MACHAMP},
    {59, SPECIES_POLIWRATH},
    {60, SPECIES_TENTACRUEL},
    {61, SPECIES_EXEGGUTOR},
    {62, SPECIES_CLOYSTER}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesUltraB;

static const SpeciesUltraB SpeciesUltraUncommon[] = {
    {0, SPECIES_DELIBIRD},
    {1, SPECIES_LICKITUNG},
    {2, SPECIES_YANMA},
    {3, SPECIES_PORYGON},
    {4, SPECIES_TOGETIC},
    {5, SPECIES_AZUMARILL},
    {6, SPECIES_MAROWAK},
    {7, SPECIES_LUNATONE},
    {8, SPECIES_SOLROCK},
    {9, SPECIES_GRANBULL},
    {10, SPECIES_HITMONLEE},
    {11, SPECIES_HITMONCHAN},
    {12, SPECIES_HITMONTOP},
    {13, SPECIES_BANETTE},
    {14, SPECIES_DUSCLOPS},
    {15, SPECIES_MR_MIME},
    {16, SPECIES_TROPIUS},
    {17, SPECIES_MAGNETON},
    {18, SPECIES_MANTINE},
    {19, SPECIES_SKARMORY},
    {20, SPECIES_WHISCASH},
    {21, SPECIES_DEWGONG},
    {22, SPECIES_MANECTRIC},
    {23, SPECIES_OCTILLERY},
    {24, SPECIES_GLALIE},
    {25, SPECIES_SLOWBRO},
    {26, SPECIES_WEEZING},
    {27, SPECIES_ELECTABUZZ},
    {28, SPECIES_SLOWKING},
    {29, SPECIES_EXPLOUD},
    {30, SPECIES_MAGMAR},
    {31, SPECIES_MUK},
    {32, SPECIES_SCYTHER},
    {33, SPECIES_PINSIR},
    {34, SPECIES_URSARING},
    {35, SPECIES_HOUNDOOM},
    {36, SPECIES_CLAYDOL},
    {37, SPECIES_AMPHAROS},
    {38, SPECIES_GARDEVOIR},
    {39, SPECIES_ABSOL},
    {40, SPECIES_CACTURNE},
    {41, SPECIES_LUDICOLO},
    {42, SPECIES_SHIFTRY},
    {43, SPECIES_POLITOED},
    {44, SPECIES_SCIZOR},
    {45, SPECIES_HERACROSS},
    {46, SPECIES_STEELIX},
    {47, SPECIES_ALTARIA},
    {48, SPECIES_RELICANTH},
    {49, SPECIES_HUNTAIL},
    {50, SPECIES_GOREBYSS}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesUltraC;

static const SpeciesUltraC SpeciesUltraRare[] = {
    {0, SPECIES_DITTO},
    {1, SPECIES_EEVEE},
    {2, SPECIES_OMANYTE},
    {3, SPECIES_KABUTO},
    {4, SPECIES_LILEEP},
    {5, SPECIES_ANORITH},
    {6, SPECIES_WOBBUFFET},
    {7, SPECIES_PUPITAR},
    {8, SPECIES_DUNSPARCE},
    {9, SPECIES_DRAGONAIR},
    {10, SPECIES_SHELGON},
    {11, SPECIES_METANG},
    {12, SPECIES_MISDREAVUS},
    {13, SPECIES_KANGASKHAN},
    {14, SPECIES_TAUROS},
    {15, SPECIES_ALAKAZAM},
    {16, SPECIES_GENGAR},
    {17, SPECIES_STARMIE},
    {18, SPECIES_FLYGON},
    {19, SPECIES_VAPOREON},
    {20, SPECIES_JOLTEON},
    {21, SPECIES_FLAREON},
    {22, SPECIES_AGGRON},
    {23, SPECIES_WALREIN},
    {24, SPECIES_CROBAT},
    {25, SPECIES_GYARADOS},
    {26, SPECIES_KINGDRA},
    {27, SPECIES_MILOTIC}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesUltraD;

static const SpeciesUltraD SpeciesUltraUltraRare[] = {
    {0, SPECIES_SHEDINJA},
    {1, SPECIES_SMEARGLE},
    {2, SPECIES_CHANSEY},
    {3, SPECIES_OMASTAR},
    {4, SPECIES_KABUTOPS},
    {5, SPECIES_CRADILY},
    {6, SPECIES_ARMALDO},
    {7, SPECIES_AERODACTYL},
    {8, SPECIES_PORYGON2},
    {9, SPECIES_VENUSAUR},
    {10, SPECIES_MEGANIUM},
    {11, SPECIES_ESPEON},
    {12, SPECIES_UMBREON},
    {13, SPECIES_BLASTOISE},
    {14, SPECIES_FERALIGATR},
    {15, SPECIES_SCEPTILE},
    {16, SPECIES_BLAZIKEN},
    {17, SPECIES_SWAMPERT},
    {18, SPECIES_CHARIZARD},
    {19, SPECIES_TYPHLOSION},
    {20, SPECIES_LAPRAS},
    {21, SPECIES_SNORLAX},
    {22, SPECIES_ARCANINE},
    {23, SPECIES_DRAGONITE},
    {24, SPECIES_TYRANITAR},
    {25, SPECIES_SALAMENCE},
    {26, SPECIES_METAGROSS},
    {27, SPECIES_SLAKING}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesMasterA;

static const SpeciesMasterA SpeciesMasterCommon[] = {
    {0, SPECIES_DITTO},
    {1, SPECIES_METANG},
    {2, SPECIES_SHELGON},
    {3, SPECIES_PUPITAR},
    {4, SPECIES_DRAGONAIR},
    {5, SPECIES_FLYGON},
    {6, SPECIES_VENUSAUR},
    {7, SPECIES_VAPOREON},
    {8, SPECIES_JOLTEON},
    {9, SPECIES_FLAREON},
    {10, SPECIES_MEGANIUM},
    {11, SPECIES_BLASTOISE},
    {12, SPECIES_FERALIGATR},
    {13, SPECIES_SCEPTILE},
    {14, SPECIES_BLAZIKEN},
    {15, SPECIES_SWAMPERT},
    {16, SPECIES_CHARIZARD},
    {17, SPECIES_TYPHLOSION}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesMasterB;

static const SpeciesMasterB SpeciesMasterUncommon[] = {
    {0, SPECIES_OMASTAR},
    {1, SPECIES_KABUTOPS},
    {2, SPECIES_CRADILY},
    {3, SPECIES_ARMALDO},
    {4, SPECIES_SHUCKLE},
    {5, SPECIES_AERODACTYL},
    {6, SPECIES_ESPEON},
    {7, SPECIES_UMBREON},
    {8, SPECIES_LAPRAS},
    {9, SPECIES_SNORLAX},
    {10, SPECIES_DRAGONITE},
    {11, SPECIES_SALAMENCE},
    {12, SPECIES_METAGROSS},
    {13, SPECIES_SHEDINJA},
    {14, SPECIES_SMEARGLE},
    {15, SPECIES_UNOWN},
    {16, SPECIES_BLISSEY}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesMasterC;

static const SpeciesMasterC SpeciesMasterRare[] = {
    {0, SPECIES_ARTICUNO},
    {1, SPECIES_ZAPDOS},
    {2, SPECIES_MOLTRES},
    {3, SPECIES_RAIKOU},
    {4, SPECIES_ENTEI},
    {5, SPECIES_SUICUNE},
    {6, SPECIES_REGIROCK},
    {7, SPECIES_REGICE},
    {8, SPECIES_REGISTEEL},
    {9, SPECIES_LATIAS},
    {10, SPECIES_LATIOS}
};

typedef struct  {
    int customNumber;
    u16 species;
} SpeciesMasterD;

static const SpeciesMasterD SpeciesMasterUltraRare[] = {
    {0, SPECIES_MEW},
    {1, SPECIES_CELEBI},
    {2, SPECIES_JIRACHI},
    {3, SPECIES_DEOXYS},
    {4, SPECIES_KYOGRE},
    {5, SPECIES_GROUDON},
    {6, SPECIES_MEWTWO},
    {7, SPECIES_LUGIA},
    {8, SPECIES_HO_OH},
    {9, SPECIES_RAYQUAZA}
};

static void ShowMessage(void)
{
	u16 bet;
	struct WindowTemplate template;
		
		
	SetWindowTemplateFields(&template, GACHA_MENUS, 17, 10, 10, 2, 0xF, 0x194);
	
    sTextWindowId = AddWindow(&template);
    FillWindowPixelBuffer(sTextWindowId, PIXEL_FILL(0));
    PutWindowTilemap(sTextWindowId);
    LoadUserWindowBorderGfx(sTextWindowId, 0x214, BG_PLTT_ID(14));
	DrawStdWindowFrame(sTextWindowId, FALSE); 
	bet = sGacha->Odds;
	ConvertUIntToDecimalStringN(gStringVar1, bet, STR_CONV_MODE_LEADING_ZEROS, 3);
	//gStringVar4[0] = '\0';
	StringExpandPlaceholders(gStringVar4, sMessageText);
    AddTextPrinterParameterized(sTextWindowId, FONT_NARROW, gStringVar4, 0, 1, 0, 0);
	CopyWindowToVram(sTextWindowId, 3);
}

static void ResetMessage(void)
{
	ClearStdWindowAndFrame(sTextWindowId, TRUE);
    RemoveWindow(sTextWindowId);
}

static void StartExitGacha(void)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    sGacha->state = GACHA_STATE_EXIT;
}

static void StartTradeScreen(void)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    sGacha->state = STATE_FADE;
}

u16 getBasicSpeciesbyNum(u16 Number) {
    int i;
    u16 totalMax;

    // Use the pre-defined totalMax values based on the rarity
    if (sGacha->Rarity == 0) { // Common
        totalMax = sGacha->Basic_CommonMax;
    }
    else if (sGacha->Rarity == 1) { // Uncommon
        totalMax = sGacha->Basic_UncommonMax;
    }
    else if (sGacha->Rarity == 2) { // Rare
        totalMax = sGacha->Basic_RareMax;
    }
    else if (sGacha->Rarity == 3) { // Ultra Rare
        totalMax = sGacha->Basic_UltraRareMax;
    }

    // Check if the provided Number is valid
    if (Number >= totalMax) {
        return -1;  // Return -1 if the Number is out of range for the list
    }

    // Now, search for the Pokémon based on its customNumber
    if (sGacha->Rarity == 0) { // Common
        for (i = 0; i < totalMax; i++) {
            if (SpeciesBasicCommon[i].customNumber == Number) {
                return SpeciesBasicCommon[i].species;
            }
        }
    }
    else if (sGacha->Rarity == 1) { // Uncommon
        for (i = 0; i < totalMax; i++) {
            if (SpeciesBasicUncommon[i].customNumber == Number) {
                return SpeciesBasicUncommon[i].species;
            }
        }
    }
    else if (sGacha->Rarity == 2) { // Rare
        for (i = 0; i < totalMax; i++) {
            if (SpeciesBasicRare[i].customNumber == Number) {
                return SpeciesBasicRare[i].species;
            }
        }
    }
    else if (sGacha->Rarity == 3) { // Ultra Rare
        for (i = 0; i < totalMax; i++) {
            if (SpeciesBasicUltraRare[i].customNumber == Number) {
                return SpeciesBasicUltraRare[i].species;
            }
        }
    }

    return -1; // Return -1 if customNumber is not found
}

u16 getGreatSpeciesbyNum(u16 Number) {
    int i;
    u16 totalMax = 0;

    // Determine the totalMax based on rarity
    if (sGacha->Rarity == 0) { // Common
        totalMax = sGacha->Great_CommonMax;
    }
    else if (sGacha->Rarity == 1) { // Uncommon
        totalMax = sGacha->Great_UncommonMax;
    }
    else if (sGacha->Rarity == 2) { // Rare
        totalMax = sGacha->Great_RareMax;
    }
    else { // Ultra Rare
        totalMax = sGacha->Great_UltraRareMax;
    }

    // Check if the provided Number is within the range
    if (Number >= totalMax) {
        return -1;  // Return -1 if out of range
    }

    // Loop through the correct array based on rarity
    if (sGacha->Rarity == 0) { // Common
        for (i = 0; i < totalMax; i++) {
            if (SpeciesGreatCommon[i].customNumber == Number) {
                return SpeciesGreatCommon[i].species;
            }
        }
    }
    else if (sGacha->Rarity == 1) { // Uncommon
        for (i = 0; i < totalMax; i++) {
            if (SpeciesGreatUncommon[i].customNumber == Number) {
                return SpeciesGreatUncommon[i].species;
            }
        }
    }
    else if (sGacha->Rarity == 2) { // Rare
        for (i = 0; i < totalMax; i++) {
            if (SpeciesGreatRare[i].customNumber == Number) {
                return SpeciesGreatRare[i].species;
            }
        }
    }
    else { // Ultra Rare
        for (i = 0; i < totalMax; i++) {
            if (SpeciesGreatUltraRare[i].customNumber == Number) {
                return SpeciesGreatUltraRare[i].species;
            }
        }
    }
    
    return -1; // Return -1 if customNumber is not found
}

u16 getUltraSpeciesbyNum(u16 Number) {
    int i;
    u16 totalMax = 0;

    // Determine the totalMax based on rarity
    if (sGacha->Rarity == 0) { // Common
        totalMax = sGacha->Ultra_CommonMax;
    }
    else if (sGacha->Rarity == 1) { // Uncommon
        totalMax = sGacha->Ultra_UncommonMax;
    }
    else if (sGacha->Rarity == 2) { // Rare
        totalMax = sGacha->Ultra_RareMax;
    }
    else { // Ultra Rare
        totalMax = sGacha->Ultra_UltraRareMax;
    }

    // Check if the provided Number is within the range
    if (Number >= totalMax) {
        return -1;  // Return -1 if out of range
    }

    // Loop through the correct array based on rarity
    if (sGacha->Rarity == 0) { // Common
        for (i = 0; i < totalMax; i++) {
            if (SpeciesUltraCommon[i].customNumber == Number) {
                return SpeciesUltraCommon[i].species;
            }
        }
    }
    else if (sGacha->Rarity == 1) { // Uncommon
        for (i = 0; i < totalMax; i++) {
            if (SpeciesUltraUncommon[i].customNumber == Number) {
                return SpeciesUltraUncommon[i].species;
            }
        }
    }
    else if (sGacha->Rarity == 2) { // Rare
        for (i = 0; i < totalMax; i++) {
            if (SpeciesUltraRare[i].customNumber == Number) {
                return SpeciesUltraRare[i].species;
            }
        }
    }
    else { // Ultra Rare
        for (i = 0; i < totalMax; i++) {
            if (SpeciesUltraUltraRare[i].customNumber == Number) {
                return SpeciesUltraUltraRare[i].species;
            }
        }
    }
    
    return -1; // Return -1 if customNumber is not found
}

u16 getMasterSpeciesbyNum(u16 Number) {
    int i;
    u16 totalMax = 0;

    // Determine the totalMax based on rarity
    if (sGacha->Rarity == 0) { // Common
        totalMax = sGacha->Master_CommonMax;
    }
    else if (sGacha->Rarity == 1) { // Uncommon
        totalMax = sGacha->Master_UncommonMax;
    }
    else if (sGacha->Rarity == 2) { // Rare
        totalMax = sGacha->Master_RareMax;
    }
    else { // Ultra Rare
        totalMax = sGacha->Master_UltraRareMax;
    }

    // Check if the provided Number is within the range
    if (Number >= totalMax) {
        return -1;  // Return -1 if out of range
    }

    // Loop through the correct array based on rarity
    if (sGacha->Rarity == 0) { // Common
        for (i = 0; i < totalMax; i++) {
            if (SpeciesMasterCommon[i].customNumber == Number) {
                return SpeciesMasterCommon[i].species;
            }
        }
    }
    else if (sGacha->Rarity == 1) { // Uncommon
        for (i = 0; i < totalMax; i++) {
            if (SpeciesMasterUncommon[i].customNumber == Number) {
                return SpeciesMasterUncommon[i].species;
            }
        }
    }
    else if (sGacha->Rarity == 2) { // Rare
        for (i = 0; i < totalMax; i++) {
            if (SpeciesMasterRare[i].customNumber == Number) {
                return SpeciesMasterRare[i].species;
            }
        }
    }
    else { // Ultra Rare
        for (i = 0; i < totalMax; i++) {
            if (SpeciesMasterUltraRare[i].customNumber == Number) {
                return SpeciesMasterUltraRare[i].species;
            }
        }
    }
    
    return -1; // Return -1 if customNumber is not found
}

u16 GetPokemon(u16 n) {
	int num;
	if (sGacha->GachaId == 1) {		 // Basic
	num = getBasicSpeciesbyNum(n);
	}
	else if (sGacha->GachaId == 2) { // Great
	num = getGreatSpeciesbyNum(n);
	}
	else if (sGacha->GachaId == 3) { // Ultra
	num = getUltraSpeciesbyNum(n);
	}
	else { 							 // Master
	num = getMasterSpeciesbyNum(n);
	}
	
	if (num == -1) {
        return 0;  // Or another default value indicating not found.
    }
	return num;
}

u8 CheckIfOwned(u16 species)
{
	u16 nationalDexNo;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	return GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT);
}

static void GetPokemonOwned(void)
{
	u16 species;
	int nationalDexNo;
	int i;
	
	sGacha->Basic_Common_Owned = 0;
	sGacha->Basic_Uncommon_Owned = 0;
	sGacha->Basic_Rare_Owned = 0;
	sGacha->Basic_UltraRare_Owned = 0;
	sGacha->Great_Common_Owned = 0;
	sGacha->Great_Uncommon_Owned = 0;
	sGacha->Great_Rare_Owned = 0;
	sGacha->Great_UltraRare_Owned = 0;
	sGacha->Ultra_Common_Owned = 0;
	sGacha->Ultra_Uncommon_Owned = 0;
	sGacha->Ultra_Rare_Owned = 0;
	sGacha->Ultra_UltraRare_Owned = 0;
	sGacha->Master_Common_Owned = 0;
	sGacha->Master_Uncommon_Owned = 0;
	sGacha->Master_Rare_Owned = 0;
	sGacha->Master_UltraRare_Owned = 0;
	
	// Basic
	for (i = 0; i < sGacha->Basic_CommonMax; i++) {
	species = SpeciesBasicCommon[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Basic_Common_Owned = (sGacha->Basic_Common_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Basic_UncommonMax; i++) {
	species = SpeciesBasicUncommon[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Basic_Uncommon_Owned = (sGacha->Basic_Uncommon_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Basic_RareMax; i++) {
	species = SpeciesBasicRare[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Basic_Rare_Owned = (sGacha->Basic_Rare_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Basic_UltraRareMax; i++) {
	species = SpeciesBasicUltraRare[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Basic_UltraRare_Owned = (sGacha->Basic_UltraRare_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	sGacha->Basic_Total_Owned = (sGacha->Basic_Common_Owned + sGacha->Basic_Uncommon_Owned + sGacha->Basic_Rare_Owned + sGacha->Basic_UltraRare_Owned);
	
	// Great
	for (i = 0; i < sGacha->Great_CommonMax; i++) {
	species = SpeciesGreatCommon[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Great_Common_Owned = (sGacha->Great_Common_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Great_UncommonMax; i++) {
	species = SpeciesGreatUncommon[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Great_Uncommon_Owned = (sGacha->Great_Uncommon_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Great_RareMax; i++) {
	species = SpeciesGreatRare[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Great_Rare_Owned = (sGacha->Great_Rare_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Great_UltraRareMax; i++) {
	species = SpeciesGreatUltraRare[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Great_UltraRare_Owned = (sGacha->Great_UltraRare_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	sGacha->Great_Total_Owned = (sGacha->Great_Common_Owned + sGacha->Great_Uncommon_Owned + sGacha->Great_Rare_Owned + sGacha->Great_UltraRare_Owned);
	
	// Ultra
	for (i = 0; i < sGacha->Ultra_CommonMax; i++) {
	species = SpeciesUltraCommon[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Ultra_Common_Owned = (sGacha->Ultra_Common_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Ultra_UncommonMax; i++) {
	species = SpeciesUltraUncommon[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Ultra_Uncommon_Owned = (sGacha->Ultra_Uncommon_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Ultra_RareMax; i++) {
	species = SpeciesUltraRare[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Ultra_Rare_Owned = (sGacha->Ultra_Rare_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Ultra_UltraRareMax; i++) {
	species = SpeciesUltraUltraRare[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Ultra_UltraRare_Owned = (sGacha->Ultra_UltraRare_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	sGacha->Ultra_Total_Owned = (sGacha->Ultra_Common_Owned + sGacha->Ultra_Uncommon_Owned + sGacha->Ultra_Rare_Owned + sGacha->Ultra_UltraRare_Owned);
	
	// Master
	for (i = 0; i < sGacha->Master_CommonMax; i++) {
	species = SpeciesMasterCommon[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Master_Common_Owned = (sGacha->Master_Common_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Master_UncommonMax; i++) {
	species = SpeciesMasterUncommon[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Master_Uncommon_Owned = (sGacha->Master_Uncommon_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Master_RareMax; i++) {
	species = SpeciesMasterRare[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Master_Rare_Owned = (sGacha->Master_Rare_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	for (i = 0; i < sGacha->Master_UltraRareMax; i++) {
	species = SpeciesMasterUltraRare[i].species;
	nationalDexNo = SpeciesToNationalPokedexNum(species);
	sGacha->Master_UltraRare_Owned = (sGacha->Master_UltraRare_Owned + GetSetPokedexFlag(nationalDexNo, FLAG_GET_CAUGHT));
	}
	sGacha->Master_Total_Owned = (sGacha->Master_Common_Owned + sGacha->Master_Uncommon_Owned + sGacha->Master_Rare_Owned + sGacha->Master_UltraRare_Owned);
}

u8 CalculateChanceForCategory(u16 owned, u16 available, u8 baseChance, u16 wager)
{
    u8 newChance;
    u8 ownedPercentage;
    u8 wagerMultiplier;
    u8 boostedChance;
    u16 minWager;

    // Determine minimum wager based on sGacha->GachaId
    switch (sGacha->GachaId)
    {
        case 1:
            minWager = 50;
            break;
        case 2:
            minWager = 250;
            break;
        case 3:
            minWager = 1000;
            break;
        case 4:
            minWager = 4500;
            break;
        default:
            minWager = 0; // Should never be 0, but failsafe
            break;
    }

    // If available Pokémon is 0, there is no chance
    if (available == 0)
        return 0;

    // Calculate the reduction in chance based on the proportion of owned Pokémon
    ownedPercentage = (owned * 100) / available;
    newChance = baseChance * (100 - ownedPercentage) / 100;

    // Ensure the wager is within the valid range
    if (wager >= minWager && owned < available)
    {
        // Normalize wager to a 0-100 range where [minWager - 9999] -> [0 - 100]
        wagerMultiplier = ((wager - minWager) * 100) / (9999 - minWager);

        // Boost the chance based on the wager multiplier, but cap it by baseChance
        boostedChance = newChance + ((baseChance - newChance) * wagerMultiplier) / 100;

        // Ensure the chance doesn't exceed baseChance
        newChance = (boostedChance > baseChance) ? baseChance : boostedChance;
    }

    return newChance;
}

// Function to determine if the player gets a new Pokémon, and the rarity
void DeterminePokemonRarityAndNewStatus(void)
{
	u16 species;
    u16 totalNotOwned;
    u8 totalOwned;
    u16 totalMax;
    u8 commonChance;
    u8 uncommonChance;
    u8 rareChance;
    u8 ultraRareChance;
	u16 newPokemonChance;
	u16 randomValue;
	u32 attempts = 1000;
	
	commonChance = sGacha->commonChance;
    uncommonChance = sGacha->uncommonChance;
    rareChance = sGacha->rareChance;
    ultraRareChance = sGacha->ultraRareChance;

    while (1) {
		randomValue = (Random() % 100);  // Generate random value between 0 and 100

		// Determine Rarity based on the chances
		if (randomValue < 50) {
			sGacha->Rarity = 0; // Common
		}
		else if (randomValue < 80) {
			sGacha->Rarity = 1; // Uncommon
		}
		else if (randomValue < 95) {
			sGacha->Rarity = 2; // Rare
		}
		else {
			sGacha->Rarity = 3; // Ultra Rare
		}

		// Get the number of available and owned Pokémon based on rarity
		if (sGacha->GachaId == 1) { // Basic
			if (sGacha->Rarity == 0) { // Common
				totalOwned = sGacha->Basic_Common_Owned;
				totalMax = sGacha->Basic_CommonMax;
			}
			else if (sGacha->Rarity == 1) { // Uncommon
				totalOwned = sGacha->Basic_Uncommon_Owned;
				totalMax = sGacha->Basic_UncommonMax;
			}
			else if (sGacha->Rarity == 2) { // Rare
				totalOwned = sGacha->Basic_Rare_Owned;
				totalMax = sGacha->Basic_RareMax;
			}
			else { // Ultra Rare
				totalOwned = sGacha->Basic_UltraRare_Owned;
				totalMax = sGacha->Basic_UltraRareMax;
			}
			sGacha->Temp_Total = totalMax;
		}
		else if (sGacha->GachaId == 2) { // Great
			if (sGacha->Rarity == 0) { // Common
				totalOwned = sGacha->Great_Common_Owned;
				totalMax = sGacha->Great_CommonMax;
			}
			else if (sGacha->Rarity == 1) { // Uncommon
				totalOwned = sGacha->Great_Uncommon_Owned;
				totalMax = sGacha->Great_UncommonMax;
			}
			else if (sGacha->Rarity == 2) { // Rare
				totalOwned = sGacha->Great_Rare_Owned;
				totalMax = sGacha->Great_RareMax;
			}
			else { // Ultra Rare
				totalOwned = sGacha->Great_UltraRare_Owned;
				totalMax = sGacha->Great_UltraRareMax;
			}
			sGacha->Temp_Total = totalMax;
		}
		else if (sGacha->GachaId == 3) { // Ultra
			if (sGacha->Rarity == 0) { // Common
				totalOwned = sGacha->Ultra_Common_Owned;
				totalMax = sGacha->Ultra_CommonMax;
			}
			else if (sGacha->Rarity == 1) { // Uncommon
				totalOwned = sGacha->Ultra_Uncommon_Owned;
				totalMax = sGacha->Ultra_UncommonMax;
			}
			else if (sGacha->Rarity == 2) { // Rare
				totalOwned = sGacha->Ultra_Rare_Owned;
				totalMax = sGacha->Ultra_RareMax;
			}
			else { // Ultra Rare
				totalOwned = sGacha->Ultra_UltraRare_Owned;
				totalMax = sGacha->Ultra_UltraRareMax;
			}
			sGacha->Temp_Total = totalMax;
		}
		else { // Master
			if (sGacha->Rarity == 0) { // Common
				totalOwned = sGacha->Master_Common_Owned;
				totalMax = sGacha->Master_CommonMax;
			}
			else if (sGacha->Rarity == 1) { // Uncommon
				totalOwned = sGacha->Master_Uncommon_Owned;
				totalMax = sGacha->Master_UncommonMax;
			}
			else if (sGacha->Rarity == 2) { // Rare
				totalOwned = sGacha->Master_Rare_Owned;
				totalMax = sGacha->Master_RareMax;
			}
			else { // Ultra Rare
				totalOwned = sGacha->Master_UltraRare_Owned;
				totalMax = sGacha->Master_UltraRareMax;
			}
			sGacha->Temp_Total = totalMax;
		}
		// Repeat similar logic for Great, Ultra, Master, etc.
		// Same logic for GachaId == 2, 3, and others

		// Calculate the total number of Pokémon the player doesn't own
		totalNotOwned = totalMax - totalOwned;

		if (totalNotOwned <= 0) {
			// If all Pokémon of the selected rarity are owned, restart the process (reroll)
			continue;  // This will make the loop restart from the beginning
		}

		// Generate a random value for the chances
		randomValue = Random() % 100;  // Generate random value between 0-99

		// Check if we should get a new Pokémon based on the odds
		if (sGacha->Odds >= randomValue) {
			// Loop until a new (not owned) Pokémon is found
			do {
				newPokemonChance = (Random() % totalMax);  // Random pull from the available pool
				species = GetPokemon(newPokemonChance);  // Get the Pokémon species based on the random value
				attempts--;
				if (attempts < 1) {
					attempts = 1000;
					randomValue = (Random() % 100);  // Generate random value between 0 and 100

					// Determine Rarity based on the chances
					if (randomValue < 50) {
						sGacha->Rarity = 0; // Common
					}
					else if (randomValue < 80) {
						sGacha->Rarity = 1; // Uncommon
					}
					else if (randomValue < 95) {
						sGacha->Rarity = 2; // Rare
					}
					else {
						sGacha->Rarity = 3; // Ultra Rare
					}
				}
				// If the Pokémon is not owned (CheckIfOwned returns 0), we found a new Pokémon
			} while (CheckIfOwned(species) == 1);  // Continue if owned (CheckIfOwned returns 1)

			// If we've broken out of the loop, we have a new Pokémon
			sGacha->IsNewPokemon = 1;  // Mark as a new Pokémon
			sGacha->CalculatedSpecies = species;  // Store the species of the new Pokémon
			break;  // Exit the loop after finding a new Pokémon
		}
		else {
			// Loop until an owned Pokémon is found
			do {
				newPokemonChance = (Random() % totalMax);  // Random pull from the available pool
				species = GetPokemon(newPokemonChance);  // Get the Pokémon species based on the random value
				attempts--;
				if (attempts < 1) {
					attempts = 1000;
					randomValue = (Random() % 100);  // Generate random value between 0 and 100

					// Determine Rarity based on the chances
					if (randomValue < 50) {
						sGacha->Rarity = 0; // Common
					}
					else if (randomValue < 80) {
						sGacha->Rarity = 1; // Uncommon
					}
					else if (randomValue < 95) {
						sGacha->Rarity = 2; // Rare
					}
					else {
						sGacha->Rarity = 3; // Ultra Rare
					}
				}

				// If the Pokémon is owned (CheckIfOwned returns 1), we have an owned Pokémon
			} while (CheckIfOwned(species) == 0);  // Continue if not owned (CheckIfOwned returns 0)

			// If we've broken out of the loop, we have an owned Pokémon
			sGacha->IsNewPokemon = 0;  // Mark as an owned Pokémon
			sGacha->CalculatedSpecies = species;  // Store the species of the owned Pokémon
			break;  // Exit the loop after finding an owned Pokémon
		}
	}
}

static void CalculatePullOdds(void)
{
	u16 totalCommonOwned;
	u16 totalCommonAvailable;
	u16 totalUncommonOwned;
	u16 totalUncommonAvailable;
	u16 totalRareOwned;
	u16 totalRareAvailable;
	u16 totalUltraRareOwned;
	u16 totalUltraRareAvailable;
	u16 wager;
	u8 commonChance;
	u8 uncommonChance;
	u8 rareChance;
	u8 ultraRareChance;
	u8 totalChance;
	
    // Constants
    const u8 COMMON_ODDS = 50;    // 50% chance for common
    const u8 UNCOMMON_ODDS = 30;  // 30% chance for uncommon
    const u8 RARE_ODDS = 15;      // 15% chance for rare
    const u8 ULTRA_RARE_ODDS = 5; // 5% chance for ultra rare
	
	if (sGacha->GachaId == 1) // Basic
	{
		// Variables to store the total numbers of owned and available Pokemon
		totalCommonOwned = sGacha->Basic_Common_Owned;
		totalCommonAvailable = sGacha->Basic_CommonMax;
		
		totalUncommonOwned = sGacha->Basic_Uncommon_Owned;
		totalUncommonAvailable = sGacha->Basic_UncommonMax;
		
		totalRareOwned = sGacha->Basic_Rare_Owned;
		totalRareAvailable = sGacha->Basic_RareMax;
		
		totalUltraRareOwned = sGacha->Basic_UltraRare_Owned;
		totalUltraRareAvailable = sGacha->Basic_UltraRareMax;

		wager = sGacha->wager;  // Player's wager (0-9999)

		// Calculate the chance for each category
		commonChance = CalculateChanceForCategory(totalCommonOwned, totalCommonAvailable, COMMON_ODDS, wager);
		uncommonChance = CalculateChanceForCategory(totalUncommonOwned, totalUncommonAvailable, UNCOMMON_ODDS, wager);
		rareChance = CalculateChanceForCategory(totalRareOwned, totalRareAvailable, RARE_ODDS, wager);
		ultraRareChance = CalculateChanceForCategory(totalUltraRareOwned, totalUltraRareAvailable, ULTRA_RARE_ODDS, wager);
	}
	else if (sGacha->GachaId == 2) // Great
	{
		// Variables to store the total numbers of owned and available Pokemon
		totalCommonOwned = sGacha->Great_Common_Owned;
		totalCommonAvailable = sGacha->Great_CommonMax;
		
		totalUncommonOwned = sGacha->Great_Uncommon_Owned;
		totalUncommonAvailable = sGacha->Great_UncommonMax;
		
		totalRareOwned = sGacha->Great_Rare_Owned;
		totalRareAvailable = sGacha->Great_RareMax;
		
		totalUltraRareOwned = sGacha->Great_UltraRare_Owned;
		totalUltraRareAvailable = sGacha->Great_UltraRareMax;

		wager = sGacha->wager;  // Player's wager (0-9999)

		// Calculate the chance for each category
		commonChance = CalculateChanceForCategory(totalCommonOwned, totalCommonAvailable, COMMON_ODDS, wager);
		uncommonChance = CalculateChanceForCategory(totalUncommonOwned, totalUncommonAvailable, UNCOMMON_ODDS, wager);
		rareChance = CalculateChanceForCategory(totalRareOwned, totalRareAvailable, RARE_ODDS, wager);
		ultraRareChance = CalculateChanceForCategory(totalUltraRareOwned, totalUltraRareAvailable, ULTRA_RARE_ODDS, wager);
	}
	else if (sGacha->GachaId == 3) // Ultra
	{
		// Variables to store the total numbers of owned and available Pokemon
		totalCommonOwned = sGacha->Ultra_Common_Owned;
		totalCommonAvailable = sGacha->Ultra_CommonMax;
		
		totalUncommonOwned = sGacha->Ultra_Uncommon_Owned;
		totalUncommonAvailable = sGacha->Ultra_UncommonMax;
		
		totalRareOwned = sGacha->Ultra_Rare_Owned;
		totalRareAvailable = sGacha->Ultra_RareMax;
		
		totalUltraRareOwned = sGacha->Ultra_UltraRare_Owned;
		totalUltraRareAvailable = sGacha->Ultra_UltraRareMax;

		wager = sGacha->wager;  // Player's wager (0-9999)

		// Calculate the chance for each category
		commonChance = CalculateChanceForCategory(totalCommonOwned, totalCommonAvailable, COMMON_ODDS, wager);
		uncommonChance = CalculateChanceForCategory(totalUncommonOwned, totalUncommonAvailable, UNCOMMON_ODDS, wager);
		rareChance = CalculateChanceForCategory(totalRareOwned, totalRareAvailable, RARE_ODDS, wager);
		ultraRareChance = CalculateChanceForCategory(totalUltraRareOwned, totalUltraRareAvailable, ULTRA_RARE_ODDS, wager);
	}
	else // Master
	{
		// Variables to store the total numbers of owned and available Pokemon
		totalCommonOwned = sGacha->Master_Common_Owned;
		totalCommonAvailable = sGacha->Master_CommonMax;
		
		totalUncommonOwned = sGacha->Master_Uncommon_Owned;
		totalUncommonAvailable = sGacha->Master_UncommonMax;
		
		totalRareOwned = sGacha->Master_Rare_Owned;
		totalRareAvailable = sGacha->Master_RareMax;
		
		totalUltraRareOwned = sGacha->Master_UltraRare_Owned;
		totalUltraRareAvailable = sGacha->Master_UltraRareMax;

		wager = sGacha->wager;  // Player's wager (0-9999)

		// Calculate the chance for each category
		commonChance = CalculateChanceForCategory(totalCommonOwned, totalCommonAvailable, COMMON_ODDS, wager);
		uncommonChance = CalculateChanceForCategory(totalUncommonOwned, totalUncommonAvailable, UNCOMMON_ODDS, wager);
		rareChance = CalculateChanceForCategory(totalRareOwned, totalRareAvailable, RARE_ODDS, wager);
		ultraRareChance = CalculateChanceForCategory(totalUltraRareOwned, totalUltraRareAvailable, ULTRA_RARE_ODDS, wager);
	}
	
	sGacha->commonChance = commonChance;
	sGacha->uncommonChance = uncommonChance;
	sGacha->rareChance = rareChance;
	sGacha->ultraRareChance = ultraRareChance;

    // Final Odds as a sum of chances
    
	totalChance = commonChance + uncommonChance + rareChance + ultraRareChance;
	if (totalChance <= 100) {
	sGacha->Odds = commonChance + uncommonChance + rareChance + ultraRareChance;
	}
	else {
	sGacha->Odds = 100;
	}	
}

static void AButton(void)
{   	
    if (sGacha->Trigger == 1)
    {
        sGacha->state = STATE_INIT_A;
    }
    else
    {
        PlaySE(SE_FAILURE);
    }
}

static void UpdateCursorPosition(s16 x)
{
    // Update cursor position based on X coordinate
    if (x == 231) {
        sGacha->cursorPosition = 3; // Ones
    } else if (x == 223) {
        sGacha->cursorPosition = 2; // Tens
    } else if (x == 215) {
        sGacha->cursorPosition = 1; // Hundreds
    } else {
        sGacha->cursorPosition = 0; // Thousands
    }
}

static void UpdateWagerDigit(int direction)
{
	u8 place;
    u16 tempwager;
    u8 wagerDigits[4];
    u16 newWager;
    u16 d;
	int i;
	u16 maxWager;
	
	place = sGacha->cursorPosition;
	d = 1000;
	tempwager = sGacha->wager;
	
	for (i = 0; i < 4; i++)
    {
		if (tempwager >= d) {
        wagerDigits[i] = tempwager / d;
		}
		else {
		wagerDigits[i] = 0;
		}

        tempwager = tempwager % d;
        d = d / 10;
    }
	maxWager = GetCoins();  // Maximum wager is the current coins
    
    // wagerDigits[0] = Thousands place
	// wagerDigits[1] = Hundreds place
    // wagerDigits[2] = Tens place
    // wagerDigits[3] = Ones place

    if (direction == 0) { // Up
        if (wagerDigits[place] == 9) {
        // Set the current digit to 0
			wagerDigits[place] = 0;
			if (place > 0) {
					place--;  // Move to the next digit on the left
			}
        }

		// Otherwise, simply increase the digit by 1
		wagerDigits[place]++;
		PlaySE(SE_SELECT);
				
        // Ensure the new wager doesn't exceed max available coins
        newWager = (wagerDigits[0] * 1000) + (wagerDigits[1] * 100) + (wagerDigits[2] * 10) + wagerDigits[3];
        if (newWager > maxWager) {
            // If the new wager exceeds available coins, revert back
            newWager = GetCoins();
        } 
            // Update the wager if it's within the limit
            sGacha->wager = newWager;
    }
    else if (direction == 2) { // Down
        if (wagerDigits[place] > 0) {
            // Decrease the digit by 1
            wagerDigits[place]--;
            sGacha->wager = (wagerDigits[0] * 1000) + (wagerDigits[1] * 100) + (wagerDigits[2] * 10) + wagerDigits[3];
			PlaySE(SE_SELECT);
        }
    }

    // Update sprite animation based on the new value
    gSprites[sGacha->ArrowsSpriteId].animNum = (wagerDigits[place] == 0) ? 1 : 0;
    SetPlayerDigits(sGacha->wager);  // Update the displayed wager
	if (sGacha->GachaId == 1)
	{
		if (sGacha->wager > 49)
		{
			ResetMessage();
			CalculatePullOdds();
			sGacha->Trigger = 1;
			//gSprites[sGacha->CTAspriteId].animPaused = FALSE;
			gSprites[sGacha->CTAspriteId].animNum = 1; // On
			ShowMessage();
		}
		else
		{
			ResetMessage();
			//CalculatePullOdds();
			sGacha->Odds = 0;
			sGacha->Trigger = 0;		
			gSprites[sGacha->CTAspriteId].animNum = 0; // Off
			//gSprites[sGacha->CTAspriteId].animPaused = TRUE;
			ShowMessage();
		}
	}
	else if (sGacha->GachaId == 2)
	{
		if (sGacha->wager > 249)
		{
			ResetMessage();
			CalculatePullOdds();
			sGacha->Trigger = 1;
			//gSprites[sGacha->CTAspriteId].animPaused = FALSE;
			gSprites[sGacha->CTAspriteId].animNum = 1; // On
			ShowMessage();
		}
		else
		{
			ResetMessage();
			//CalculatePullOdds();
			sGacha->Odds = 0;
			sGacha->Trigger = 0;		
			gSprites[sGacha->CTAspriteId].animNum = 0; // Off
			//gSprites[sGacha->CTAspriteId].animPaused = TRUE;
			ShowMessage();
		}
	}
	else if (sGacha->GachaId == 3)
	{
		if (sGacha->wager > 999)
		{
			ResetMessage();
			CalculatePullOdds();
			sGacha->Trigger = 1;
			//gSprites[sGacha->CTAspriteId].animPaused = FALSE;
			gSprites[sGacha->CTAspriteId].animNum = 1; // On
			ShowMessage();
		}
		else
		{
			ResetMessage();
			//CalculatePullOdds();
			sGacha->Odds = 0;
			sGacha->Trigger = 0;		
			gSprites[sGacha->CTAspriteId].animNum = 0; // Off
			//gSprites[sGacha->CTAspriteId].animPaused = TRUE;
			ShowMessage();
		}
	}
	else
	{
		if (sGacha->wager > 4499)
		{
			ResetMessage();
			CalculatePullOdds();
			sGacha->Trigger = 1;
			//gSprites[sGacha->CTAspriteId].animPaused = FALSE;
			gSprites[sGacha->CTAspriteId].animNum = 1; // On
			ShowMessage();
		}
		else
		{
			ResetMessage();
			//CalculatePullOdds();
			sGacha->Odds = 0;
			sGacha->Trigger = 0;		
			gSprites[sGacha->CTAspriteId].animNum = 0; // Off
			//gSprites[sGacha->CTAspriteId].animPaused = TRUE;
			ShowMessage();
		}
	}
}

static void MoveCursor(int direction)
{
    struct Sprite *cursorSprite = &gSprites[sGacha->ArrowsSpriteId];
    int curX = cursorSprite->x;
    int destX = curX;
    u16 num = sGacha->wager;
    
    // Move cursor left or right (X axis)
    if (direction == 1 || direction == 3) { // Right or Left
        if (direction == 1 && curX < 231) {
            destX = curX + 8;
            PlaySE(SE_SELECT);
        } else if (direction == 3 && curX > 207) {
            destX = curX - 8;
            PlaySE(SE_SELECT);
        }
        
        cursorSprite->x = destX;
        UpdateCursorPosition(gSprites[sGacha->ArrowsSpriteId].x);  // Update cursor position based on X coordinate
    }
    
    // Move cursor up or down (change wager digit)
    else if (direction == 0 || direction == 2) { // Up or Down
        UpdateWagerDigit(direction); // Update the corresponding digit
    }
}

static void ExitGacha(void)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        FREE_AND_SET_NULL(sGacha);
		FREE_AND_SET_NULL(sTradeAnim);
    }
}

static void EggHatchPrintMessage(u8 windowId, u8 *string, u8 x, u8 y, u8 speed)
{
    FillWindowPixelBuffer(windowId, PIXEL_FILL(15));
    sGacha->textColor[0] = 0;
    sGacha->textColor[1] = 5;
    sGacha->textColor[2] = 6;
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, x, y, 0, 0, sGacha->textColor, speed, string);
}

static void HandleInput_GachaComplete(void)
{
    if (IsFanfareTaskInactive())
    {
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            gSpecialVar_Result = 1;
            sGacha->state = GACHA_STATE_START_EXIT;
        }
    }
}

static void HandleInput(void)
{
	if (sGacha->Input == 0) 
	{
		if (JOY_NEW(A_BUTTON))
		{
			AButton();
		}
		else if (JOY_NEW(B_BUTTON))
		{
			if (sGacha->exitToggle == 0) {
			sGacha->state = GACHA_STATE_START_EXIT;
			}
		}
		else if (JOY_NEW(DPAD_UP))
		{
			MoveCursor(0);
		}
		else if (JOY_NEW(DPAD_RIGHT))
		{
			MoveCursor(1);
		}
		else if (JOY_NEW(DPAD_DOWN))
		{
			MoveCursor(2);
		}
		else if (JOY_NEW(DPAD_LEFT))
		{
			MoveCursor(3);
		}
	}
}

u8 GenerateRandomIV(void) {
    return (Random() % 17) + 15;  // Random value between 15 and 31
}

static void Fade(void)
{
	BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
	sGacha->state = STATE_POKEBALL_INIT;
}

static void RemoveGarbage(void)
{
	DestroySpriteAndFreeResources(&gSprites[sGacha->CreditSpriteIds[0]]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->CreditSpriteIds[1]]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->CreditSpriteIds[2]]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->CreditSpriteIds[3]]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->PlayerSpriteIds[0]]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->PlayerSpriteIds[1]]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->PlayerSpriteIds[2]]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->PlayerSpriteIds[3]]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->KnobSpriteId]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->DigitalTextSpriteId]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->LotteryJPNspriteId]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->CreditMenu1Id]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->CreditMenu2Id]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->PokemonOneSpriteId]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->PokemonTwoSpriteId]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->PokemonThreeSpriteId]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->ArrowsSpriteId]);
	DestroySpriteAndFreeResources(&gSprites[sGacha->CTAspriteId]);
	ResetMessage();
	sTradeAnim->bg1vofs = 0;
	sTradeAnim->bg1hofs = 0;
		
	sTradeAnim->bg2vofs = 0;
	sTradeAnim->bg2hofs = 0;
	SetGpuReg(REG_OFFSET_BLDCNT, 0);
	SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(2) |
								 BGCNT_CHARBASE(1) |
								 BGCNT_16COLOR |
								 BGCNT_SCREENBASE(18) |
								 BGCNT_TXT512x256);
	LoadPalette(gTradeGba2_Pal, BG_PLTT_ID(1), 3 * PLTT_SIZE_4BPP);
	DmaCopyLarge16(3, gTradeGba_Gfx, (void *) BG_CHAR_ADDR(1), 0x1420, 0x1000);
	DmaCopy16Defvars(3, gTrade_Tilemap, (void *) BG_SCREEN_ADDR(18), 0x1000);	
	
	gPaletteFade.bufferTransferDisabled = TRUE;
	gPaletteFade.bufferTransferDisabled = FALSE;
	BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(GachaVBlankCallback);
}

static void GachaSetMonNickname(void)
{
    SetMonData(&sGacha->GachaMon, MON_DATA_NICKNAME, gStringVar3);
    FreeMonSpritesGfx();
    sGacha->state = GACHA_STATE_START_EXIT;
}

void ShowFinalMessage(void)
{
	struct WindowTemplate template;

	SetWindowTemplateFields(&template, 1, 2, 15, 26, 4, 0xF, 0x194);
	
    sTextWindowId = AddWindow(&template);
    FillWindowPixelBuffer(sTextWindowId, PIXEL_FILL(0));
    PutWindowTilemap(sTextWindowId);
    LoadUserWindowBorderGfx(sTextWindowId, 0x214, BG_PLTT_ID(14));
	DrawStdWindowFrame(sTextWindowId, FALSE); 
	StringCopy(gStringVar1, gSpeciesNames[sGacha->CalculatedSpecies]);
    StringExpandPlaceholders(gStringVar4, gText_FromGacha);
    AddTextPrinterParameterized(sTextWindowId, FONT_NORMAL, gStringVar4, 0, 1, 0, 0);
	CopyWindowToVram(sTextWindowId, 3);
}


static void GachaMain(u8 taskId)
{
    u16 species;
    u8 fixedIV;
    u8 hasFixedPersonality = TRUE;
    u32 fixedPersonality;
    u8 trainerId[TRAINER_ID_LENGTH] = {0};  // Assuming this is set elsewhere
    u32 playerId = GetTrainerId(trainerId);
    u8 otIdType = OT_ID_PLAYER_ID;
    u32 fixedOtId = playerId;
	u16 level;
	u8 check;
	u8 gender;
    u32 personality;
	u8 party;
	int pos = 0;
	//struct Pokemon *mon = NULL;
	struct Pokemon mon;
	
	pos = B_POSITION_OPPONENT_RIGHT;
	//struct SpriteSheet s;
	
	check = 0;
	
	if (FlagGet(FLAG_IS_CHAMPION) == TRUE)
	{
		level = (Random() % 30) + 40;
	}
	else if (FlagGet(FLAG_BADGE08_GET) == TRUE)
	{
		level = (Random() % 15) + 36;
	}
	else if (FlagGet(FLAG_BADGE07_GET) == TRUE)
	{
		level = (Random() % 9) + 28;
	}
	else if (FlagGet(FLAG_BADGE06_GET) == TRUE)
	{
		level = (Random() % 10) + 21;
	}
	else if (FlagGet(FLAG_BADGE05_GET) == TRUE)
	{
		level = (Random() % 10) + 19;
	}
	else if (FlagGet(FLAG_BADGE04_GET) == TRUE)
	{
		level = (Random() % 6) + 18;
	}
	else if (FlagGet(FLAG_BADGE03_GET) == TRUE)
	{
		level = (Random() % 8) + 13;
	}
	else if (FlagGet(FLAG_BADGE02_GET) == TRUE)
	{
		level = (Random() % 6) + 7;
	}
	else if (FlagGet(FLAG_BADGE01_GET) == TRUE)
	{
		level = (Random() % 7) + 5;
	}
	else
	{
		level = (Random() % 5) + 2;
	}
	
    switch (sGacha->state)
    {
    case GACHA_STATE_INIT:
        if (!gPaletteFade.active) {
            sGacha->state = GACHA_STATE_PROCESS_INPUT;
		}
        break;
    case GACHA_STATE_PROCESS_INPUT:
        HandleInput();
        break;
    case GACHA_STATE_COMPLETED_WAIT_FOR_SOUND:
        if (IsSEPlaying())
            break;

        PlayFanfare(MUS_SLOTS_WIN);
        sGacha->state = GACHA_STATE_PROCESS_COMPLETED_INPUT;
    case GACHA_STATE_PROCESS_COMPLETED_INPUT:
        HandleInput_GachaComplete();
        break;
    case GACHA_STATE_START_EXIT:
        StartExitGacha();
        break;
    case GACHA_STATE_EXIT:
        ExitGacha();
        break;
	case STATE_INIT_A: // Initial state
		sGacha->Input = 1;
		DeterminePokemonRarityAndNewStatus();
		PlaySE(SE_SHOP);
		RemoveCoins(sGacha->wager);
		sGacha->wager = 0;
		ResetMessage();
		gSprites[sGacha->CTAspriteId].animNum = 0;
		gSprites[sGacha->ArrowsSpriteId].invisible = TRUE;
		SetCreditDigits(GetCoins());
		SetPlayerDigits(sGacha->wager);
        sGacha->waitTimer = 30;  // Set the timer
        sGacha->state = STATE_TIMER_1;  // Move to next state
        break;

    case STATE_TIMER_1: // Waiting for timer to expire
        if (sGacha->waitTimer > 0) {
            sGacha->waitTimer--;  // Decrease timer
        } else {
            sGacha->state = STATE_TWIST;  // Transition to next state when the timer is done
        }
        break;

    case STATE_TWIST: // After timer expires, proceed with animation
        PlaySE(SE_VEND);
        gSprites[sGacha->KnobSpriteId].animNum = 1;
        sGacha->state = STATE_TIMER_2;  // Move to the next state after animation starts
        break;

    case STATE_TIMER_2: // Handle the next part of the delay or action
        // (You can add another waiting period if needed)
        sGacha->waitTimer = 50;  // Set the next timer
        sGacha->state = STATE_INIT_GIVE;  // Move to next state
        break;

    case STATE_INIT_GIVE: // Final state
        if (sGacha->waitTimer > 0) {
            sGacha->waitTimer--;  // Decrease timer
        } else {
            sGacha->state = STATE_SHAKE_1;  // Final action after timer
		}
		break;	
		
	case STATE_SHAKE_1: // After timer expires, proceed with animation
        PlaySE(SE_BREAKABLE_DOOR);
        Shake1();
        sGacha->state = STATE_TIMER_3;  // Move to the next state after animation starts
        break;

    case STATE_TIMER_3: // Handle the next part of the delay or action
        // (You can add another waiting period if needed)
        sGacha->waitTimer = 3;  // Set the next timer
        sGacha->state = STATE_INIT_SHAKE_2;  // Move to next state
        break;

    case STATE_INIT_SHAKE_2: // Final state
        if (sGacha->waitTimer > 0) {
            sGacha->waitTimer--;  // Decrease timer
        } else {
            sGacha->state = STATE_SHAKE_2;  // Final action after timer
		}
		break;	
	case STATE_SHAKE_2: // After timer expires, proceed with animation
        //PlaySE(SE_BREAKABLE_DOOR);
        Shake2();
        sGacha->state = STATE_TIMER_4;  // Move to the next state after animation starts
        break;

    case STATE_TIMER_4: // Handle the next part of the delay or action
        // (You can add another waiting period if needed)
        sGacha->waitTimer = 3;  // Set the next timer
        sGacha->state = STATE_INIT_SHAKE_3;  // Move to next state
        break;

    case STATE_INIT_SHAKE_3: // Final state
        if (sGacha->waitTimer > 0) {
            sGacha->waitTimer--;  // Decrease timer
        } else {
			BGSetup();
			sGacha->waitTimer = 20;
            sGacha->state = STATE_TIMER_5;  // Final action after timer
		}
		break;		
		
	case STATE_TIMER_5: // After timer expires, proceed with animation
        if (sGacha->waitTimer > 0) {
            sGacha->waitTimer--;  // Decrease timer
        } else {
        sGacha->state = STATE_GIVE;  // Move to the next state after animation starts
		}
        break;	
		
	case STATE_GIVE:
		StartTradeScreen();
		break;
	case STATE_FADE:
		if (!gPaletteFade.active) {
			BGRed();
		sGacha->state = STATE_POKEBALL_INIT; }
		break;
	case STATE_POKEBALL_INIT:
		RemoveGarbage();
		sGacha->state++;
		break;	
	case STATE_POKEBALL_PROCESS:
		if (!gPaletteFade.active) {
		sGacha->state = STATE_POKEBALL_ARRIVE; }
		break;
	case STATE_POKEBALL_ARRIVE:	
		LoadSpriteSheet(&sPokeBallSpriteSheet);
        LoadSpritePalette(&sPokeBallSpritePalette);
		sGacha->bouncingPokeballSpriteId = CreateSprite(&sSpriteTemplate_Pokeball, 120, -8, 0);
        gSprites[sGacha->bouncingPokeballSpriteId].data[3] = 74;
        gSprites[sGacha->bouncingPokeballSpriteId].callback = SpriteCB_BouncingPokeballArrive;
        StartSpriteAnim(&gSprites[sGacha->bouncingPokeballSpriteId], 1);
        StartSpriteAffineAnim(&gSprites[sGacha->bouncingPokeballSpriteId], 2);
        BlendPalettes(1 << (16 + gSprites[sGacha->bouncingPokeballSpriteId].oam.paletteNum), 16, RGB_WHITEALPHA);
        sGacha->state++;
        sGacha->timer = 0;
		break;
	case STATE_FADE_POKEBALL_TO_NORMAL:
		BeginNormalPaletteFade(1 << (16 + gSprites[sGacha->bouncingPokeballSpriteId].oam.paletteNum), 1, 16, 0, RGB_WHITEALPHA);
        sGacha->state++;
		break;
	case STATE_POKEBALL_ARRIVE_WAIT:		
		if (gSprites[sGacha->bouncingPokeballSpriteId].callback == SpriteCallbackDummy)
			{	CreateMon(&mon, sGacha->CalculatedSpecies, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
				party = GiveMonToPlayer(&mon);
				GetSetPokedexFlag(sGacha->CalculatedSpecies, FLAG_SET_SEEN);
				HandleSetPokedexFlag(sGacha->CalculatedSpecies, FLAG_SET_CAUGHT, GetMonData(&mon, MON_DATA_PERSONALITY));
				LoadCompressedSpritePalette(GetMonSpritePalStruct(&mon));
				SetMultiuseSpriteTemplateToPokemon(sGacha->CalculatedSpecies, pos);
				sGacha->monSpriteId = CreateMonPicSprite_Affine(sGacha->CalculatedSpecies, SHINY_ODDS, 0, MON_PIC_AFFINE_FRONT, 120, 60, 14, TAG_NONE);
				gSprites[sGacha->monSpriteId].callback = SpriteCB_Null;
				gSprites[sGacha->monSpriteId].oam.priority = 0;
				gSprites[sGacha->monSpriteId].invisible = TRUE;
				HandleLoadSpecialPokePic_2(&gMonFrontPicTable[sGacha->CalculatedSpecies],
											gMonSpritesGfxPtr->sprites.ptr[pos],
											sGacha->CalculatedSpecies,
											GetMonData(&mon, MON_DATA_PERSONALITY));
				sGacha->state++;
			}
		break;
	case STATE_SHOW_NEW_MON:
		
		gSprites[sGacha->monSpriteId].x = 120;
        gSprites[sGacha->monSpriteId].y = gMonFrontPicCoords[sGacha->CalculatedSpecies].y_offset + 56;
        gSprites[sGacha->monSpriteId].x2 = 0;
        gSprites[sGacha->monSpriteId].y2 = 0;
        StartSpriteAnim(&gSprites[sGacha->monSpriteId], 0);
        CreatePokeballSpriteToReleaseMon(sGacha->monSpriteId, gSprites[sGacha->monSpriteId].oam.paletteNum, 120, 84, 2, 1, 20, PALETTES_BG | (0xF << 16), sGacha->CalculatedSpecies);
        FreeSpriteOamMatrix(&gSprites[sGacha->bouncingPokeballSpriteId]);
        DestroySprite(&gSprites[sGacha->bouncingPokeballSpriteId]);
        sGacha->state++;
		break;
    case STATE_NEW_MON_MSG:
        // Wait for Pokémon's front sprite animation
        if (gSprites[sGacha->monSpriteId].callback == SpriteCallbackDummy)
            sGacha->state++;
        break;
    case NEW_1:
        // "{mon} hatched from egg" message/fanfare
		ShowFinalMessage();
        //EggHatchPrintMessage(0, gStringVar4, 0, 3, TEXT_SKIP_DRAW);
        PlayFanfare(MUS_EVOLVED);
        sGacha->state++;
        //PutWindowTilemap(0);
        //CopyWindowToVram(0, COPYWIN_FULL);
        break;
    case NEW_2:
        if (IsFanfareTaskInactive())
            sGacha->state++;
        break;
    case NEW_3: // Twice?
        if (IsFanfareTaskInactive())
            sGacha->state++;
        break;
    case NEW_4:
        // Ready the nickname prompt
		if (FlagGet(FLAG_SYS_POKEMON_GET) == FALSE){
			FlagSet(FLAG_SYS_POKEMON_GET);
		}
		sGacha->state = GACHA_STATE_START_EXIT;
        break;
    }
}

static void InitTradeScreen(void)
{
	//SetVBlankCallback(NULL);
    //ResetAllBgsCoordinates();
    //ResetVramOamAndBgCntRegs();
    //ResetBgsAndClearDma3BusyFlags(0);
    //ResetTempTileDataBuffers();
}

static void InitGachaScreen(void)
{	
	sGacha->GachaId = gSpecialVar_0x8004;

    SetVBlankCallback(NULL);
    ResetAllBgsCoordinates();
    ResetVramOamAndBgCntRegs();
    ResetBgsAndClearDma3BusyFlags(0);
    ResetTempTileDataBuffers();

	BGSetup();

	ResetSpriteData();
    FreeAllSpritePalettes();
	LoadSpritePalettes(sSpritePalettes2);
	
	if (sGacha->GachaId == 1) { // Basic
	CreateHoppip();
	}
	else if (sGacha->GachaId == 2) { // Great
	CreatePhanpy();
	}
	else if (sGacha->GachaId == 3) { // Ultra
	CreateTeddiursa();
	}
	else { // Master
	CreateBelossom();
	}
	CreateArrows();
	CreateCTA();
	CreateDigitalText();	
	CreateKnob();
	CreateCreditSprites();
	CreatePlayerSprites();
	SetCreditDigits(GetCoins());
	SetPlayerDigits(0);	
	CreateCreditMenu();	
	CreatePlayerMenu();
	CreateLotteryJPN();
	
	sGacha->Odds = 0;
	InitWindows(sGachaWinTemplates);
	LoadPalette(GetTextWindowPalette(2), 11 * 16, 32);
	ShowMessage();
	
	sGacha->exitToggle = 0;
	UpdateCursorPosition(gSprites[sGacha->ArrowsSpriteId].x);
	sGacha->Basic_CommonMax = 37;
	sGacha->Basic_UncommonMax = 28;
	sGacha->Basic_RareMax = 16;
	sGacha->Basic_UltraRareMax = 11;
	sGacha->Great_CommonMax = 65;
	sGacha->Great_UncommonMax = 52;
	sGacha->Great_RareMax = 41;
	sGacha->Great_UltraRareMax = 18;
	sGacha->Ultra_CommonMax = 63;
	sGacha->Ultra_UncommonMax = 51;
	sGacha->Ultra_RareMax = 28;
	sGacha->Ultra_UltraRareMax = 28;
	sGacha->Master_CommonMax = 18;
	sGacha->Master_UncommonMax = 17;
	sGacha->Master_RareMax = 11;
	sGacha->Master_UltraRareMax = 10;
	sGacha->gachaState = 0;
	sGacha->waitTimer = 0;
	sGacha->Input = 0;
	sGacha->Basic_Total_Max = (sGacha->Basic_CommonMax + sGacha->Basic_UncommonMax + sGacha->Basic_RareMax + sGacha->Basic_UltraRareMax);
	sGacha->Great_Total_Max = (sGacha->Great_CommonMax + sGacha->Great_UncommonMax + sGacha->Great_RareMax + sGacha->Great_UltraRareMax);
	sGacha->Ultra_Total_Max = (sGacha->Ultra_CommonMax + sGacha->Ultra_UncommonMax + sGacha->Ultra_RareMax + sGacha->Ultra_UltraRareMax);
	sGacha->Master_Total_Max = (sGacha->Master_CommonMax + sGacha->Master_UncommonMax + sGacha->Master_RareMax + sGacha->Master_UltraRareMax);
	GetPokemonOwned();
	
	CopyBgTilemapBufferToVram(GACHA_BG_BASE);
	CopyBgTilemapBufferToVram(GACHA_MENUS);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_BG2_ON);
    ShowBg(GACHA_BG_BASE);
	ShowBg(GACHA_MENUS);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(GachaVBlankCallback);
    SetMainCallback2(GachaMainCallback);
    CreateTask(GachaMain, 1);
}

