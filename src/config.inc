#ifndef PORT_TYPE
#define PORT_TYPE uint16_t
#endif

#ifndef INT_TYPE
#define INT_TYPE int32_t
#endif

#ifndef STR_TYPE
#define STR_TYPE QString
#endif

#ifndef CONFIG_DEFINE
#define CONFIG_DEFINE(Type, Name, Default, ValueWrapper, ... /* Json index */)
#endif

#ifndef GET_STR
#define GET_STR(X) X
#endif

#ifndef GET_INT
#define GET_INT(X) X
#endif

#ifndef GET_QJSONARRAY
#define GET_QJSONARRAY(X) X
#endif

CONFIG_DEFINE(PORT_TYPE, AutoSyncVariablesOnConnect, 1, GET_INT)
CONFIG_DEFINE(PORT_TYPE, AutoConnect, 0, GET_INT, "Rcon")
CONFIG_DEFINE(PORT_TYPE, ConfigVersion, CONFIG_VERSION, GET_INT)
CONFIG_DEFINE(PORT_TYPE, RconPort, 27015, GET_INT, "Rcon")
CONFIG_DEFINE(PORT_TYPE, SSHPort, 21, GET_INT, "SSH")

CONFIG_DEFINE(STR_TYPE, RconAddress, "127.0.0.1", GET_STR, "Rcon")
CONFIG_DEFINE(STR_TYPE, RconPassword, "example", GET_STR, "Rcon")

CONFIG_DEFINE(STR_TYPE, SSHAddress, "127.0.0.1", GET_STR, "SSH")
CONFIG_DEFINE(STR_TYPE, SSHPassword, "unimplemented", GET_STR, "SSH")

CONFIG_DEFINE(QJsonArray, BannedPlayers, {}, GET_QJSONARRAY)

#ifndef CONFIG_CURRENT
#define CONFIG_CURRENT(T, N, D, W) CONFIG_DEFINE(T, N, D, W, "Presets", "_CurrentPreset")
#endif

CONFIG_CURRENT(STR_TYPE, ServerName, "Default Server Name", GET_STR)
CONFIG_CURRENT(STR_TYPE, Password, "", GET_STR)
CONFIG_CURRENT(STR_TYPE, Map, "_3x2_Urban_roads_Krupki_LD_10v10", GET_STR)
CONFIG_CURRENT(INT_TYPE, NbMinPlayer, 20, GET_INT)
CONFIG_CURRENT(INT_TYPE, NbMaxPlayer, 20, GET_INT)
CONFIG_CURRENT(INT_TYPE, MaxTeamSize, 10, GET_INT)
CONFIG_CURRENT(INT_TYPE, Seed, 0, GET_INT)
CONFIG_CURRENT(INT_TYPE, GameType, 0, GET_INT)
CONFIG_CURRENT(INT_TYPE, InitMoney, 1500, GET_INT)
CONFIG_CURRENT(INT_TYPE, ScoreLimit, 5000, GET_INT)
CONFIG_CURRENT(INT_TYPE, TimeLimit, 2400, GET_INT)
CONFIG_CURRENT(INT_TYPE, IncomeRate, 3, GET_INT)
CONFIG_CURRENT(STR_TYPE, DivisionTagFilter, "", GET_STR)
CONFIG_CURRENT(INT_TYPE, MapRotationType, 0, GET_INT)
CONFIG_CURRENT(INT_TYPE, VictoryCond, 2, GET_INT)
CONFIG_CURRENT(INT_TYPE, CombatRule, 1, GET_INT)
CONFIG_CURRENT(INT_TYPE, InverseSpawnPoints, 0, GET_INT)
CONFIG_CURRENT(INT_TYPE, DebriefingTimeMax, 180, GET_INT)
CONFIG_CURRENT(INT_TYPE, WarmupCountdown, 60, GET_INT)
CONFIG_CURRENT(INT_TYPE, LoadingTimeMax, 120, GET_INT)
CONFIG_CURRENT(INT_TYPE, DeploiementTimeMax, 300, GET_INT)
CONFIG_CURRENT(INT_TYPE, PhaseADuration, 300, GET_INT)
CONFIG_CURRENT(INT_TYPE, PhaseBDuration, 300, GET_INT)
CONFIG_CURRENT(INT_TYPE, AllowObservers, 0, GET_INT)
CONFIG_CURRENT(INT_TYPE, ObserverDelay, 120, GET_INT)
CONFIG_CURRENT(INT_TYPE, CoopVsAI, 0, GET_INT)
CONFIG_CURRENT(INT_TYPE, AutoFillAI, 1, GET_INT)
CONFIG_CURRENT(INT_TYPE, DeltaTimeCheckAutoFillAI, 10, GET_INT)
CONFIG_CURRENT(STR_TYPE, ModList, "", GET_STR)
CONFIG_CURRENT(STR_TYPE, ModTagList, "", GET_STR)

#undef CONFIG_CURRENT

#ifndef CONFIG_SAVE_ONLY
#define CONFIG_SAVE_ONLY(...)
#endif

CONFIG_SAVE_ONLY(INT_TYPE, AutoFillAI, 0, GET_INT, "Presets", "WaitPlayers")
CONFIG_SAVE_ONLY(INT_TYPE, WarmupCountdown, 300, GET_INT, "Presets", "WaitPlayers")
CONFIG_SAVE_ONLY(INT_TYPE, AutoFillAI, 1, GET_INT, "Presets", "ReadyToLaunch")
CONFIG_SAVE_ONLY(INT_TYPE, WarmupCountdown, 30, GET_INT, "Presets", "ReadyToLaunch")

#ifndef CONFIG_OPTIONS
#define CONFIG_OPTIONS(Name, PossibleValue, Comment, ...)
#endif

// TODO: Localization

CONFIG_OPTIONS(GameType, 0, "Allies vs Axis (Default)")
CONFIG_OPTIONS(GameType, 1, "Allies vs Allies")
CONFIG_OPTIONS(GameType, 2, "Axis vs Axis")
CONFIG_OPTIONS(IncomeRate, 0, "None")
CONFIG_OPTIONS(IncomeRate, 1, "Very Low")
CONFIG_OPTIONS(IncomeRate, 2, "Low")
CONFIG_OPTIONS(IncomeRate, 3, "Normal (Default)")
CONFIG_OPTIONS(IncomeRate, 4, "High")
CONFIG_OPTIONS(IncomeRate, 5, "Very High")

CONFIG_OPTIONS(CombatRule, 1, "Destruction")
CONFIG_OPTIONS(CombatRule, 2, "Conquest")
CONFIG_OPTIONS(VictoryCond, 2, "Normal")
CONFIG_OPTIONS(VictoryCond, 3, "Close Quarter")
CONFIG_OPTIONS(VictoryCond, 5, "Breakthrough")

CONFIG_OPTIONS(DivisionTagFilter, "", "Unrestricted (Default)")
CONFIG_OPTIONS(DivisionTagFilter, "East", "Only Eastern Front Decks")
CONFIG_OPTIONS(DivisionTagFilter, "West", "Only Western Front Decks")

CONFIG_OPTIONS(InverseSpawnPoints, 0, "Normal (Default)")
CONFIG_OPTIONS(InverseSpawnPoints, 1, "Inverted")

CONFIG_OPTIONS(AllowObservers, 0, "Disabled")
CONFIG_OPTIONS(AllowObservers, 1, "Enabled (Default)")

CONFIG_OPTIONS(CoopVsAI, 0, "Disabled (Default)")
CONFIG_OPTIONS(CoopVsAI, 1, "Force Players to First Team")
CONFIG_OPTIONS(CoopVsAI, 2, "Force Players to Second Team")
CONFIG_OPTIONS(AutoFillAI, 0, "Disabled (Default)")
CONFIG_OPTIONS(AutoFillAI, 1, "Enabled")

CONFIG_OPTIONS(MapRotationType, 0, "Disabled (Default)")
CONFIG_OPTIONS(MapRotationType, 1, "Cycle")
CONFIG_OPTIONS(MapRotationType, 2, "Shuffle")

CONFIG_OPTIONS(Map, "_2x1_Proto_levelBuild_Orsha_N_LD_1v1", "Orsha, north #1v1")
CONFIG_OPTIONS(Map, "_2x2_Foret_Tsel_LD_1v1", "Tsel #1v1")
CONFIG_OPTIONS(Map, "_2x2_Kostritsa_LD_1v1", "Kostritsa #1v1")
CONFIG_OPTIONS(Map, "_2x2_Lacs_Sianno_LD_1v1", "Sianno #1v1")
CONFIG_OPTIONS(Map, "_2x2_Lenina_LD_1v1", "Lenina #1v1")
CONFIG_OPTIONS(Map, "_2x2_Ostrowno_LD_1v1", "Ostrowno #1v1")
CONFIG_OPTIONS(Map, "_2x2_Plateau_Central_Orsha_E_LD_1v1", "Orsha, east #1v1")
CONFIG_OPTIONS(Map, "_2x2_River_Swamp_Krupa_LD_1v1", "Krupa #1v1")
CONFIG_OPTIONS(Map, "_2x2_Shchedrin_LD_1v1", "Shchedrin #1v1")
CONFIG_OPTIONS(Map, "_2x2_Slutsk_E_LD_1v1", "Slutsk, east #1v1")
CONFIG_OPTIONS(Map, "_2x2_Slutsk_LD_1v1", "Slutsk #1v1")
CONFIG_OPTIONS(Map, "_2x2_Slutsk_W_LD_1v1", "Slutsk, west #1v1")
CONFIG_OPTIONS(Map, "_2x2_Urban_River_Bobr_LD_1v1", "Bobr #1v1")
CONFIG_OPTIONS(Map, "_2x2_Ville_Centrale_Haroshaje_LD_1v1", "Haroshaje #1v1")
CONFIG_OPTIONS(Map, "_3x2_Astrouna_Novka_LD_2v2", "Novka #2v2")
CONFIG_OPTIONS(Map, "_3x2_Beshankovichy_LD_2v2", "Beshankovichy #2v2")
CONFIG_OPTIONS(Map, "_3x2_Bridges_Smolyany_LD_3v3", "Smolyany #3v3")
CONFIG_OPTIONS(Map, "_3x2_East_Vitebsk_LD_2v2", "Vitebsk, east #2v2")
CONFIG_OPTIONS(Map, "_3x2_Highway_LD", "Autobahn zur Holle")
CONFIG_OPTIONS(Map, "_3x2_Ilomantsi_LD_1v1", "Ilomantsi #1v1")
CONFIG_OPTIONS(Map, "_3x2_Ilomantsi_LD_2v2", "Ilomantsi #2v2")
CONFIG_OPTIONS(Map, "_3x2_Island_Losnica_LD_2v2", "Losnica #2v2")
CONFIG_OPTIONS(Map, "_3x2_Lenina_LD_2v2", "Lenina #2v2")
CONFIG_OPTIONS(Map, "_3x2_Lenina_LD_3v3", "Lenina #3v3")
CONFIG_OPTIONS(Map, "_3x2_Lipen_LD_3v3", "Lipen #3v3")
CONFIG_OPTIONS(Map, "_3x2_Lyakhavichy_LD_3v3", "Lyakhavichy #3v3")
CONFIG_OPTIONS(Map, "_3x2_Marecages_Naratch_lake_LD_3v3", "Naratch lake #3v3")
CONFIG_OPTIONS(Map, "_3x2_Mogilev_LD_10v10", "Mogilev #10vs10")
CONFIG_OPTIONS(Map, "_3x2_Mogilev_LD_3v3", "Mogilev #3v3")
CONFIG_OPTIONS(Map, "_3x2_Ostrowno_LD_2v2", "Ostrowno #2v2")
CONFIG_OPTIONS(Map, "_3x2_Ostrowno_LD_3v3", "Ostrowno #3v3")
CONFIG_OPTIONS(Map, "_3x2_Rivers_Pleshchenitsy_S_LD_3v3", "Pleshchenitsy, south #3v3")
CONFIG_OPTIONS(Map, "_3x2_Shchedrin_LD_2v2", "Shchedrin #2v2")
CONFIG_OPTIONS(Map, "_3x2_Shchedrin_LD_3v3", "Shchedrin #3v3")
CONFIG_OPTIONS(Map, "_3x2_Siedlce_LD_1v1", "Siedlce #1v1")
CONFIG_OPTIONS(Map, "_3x2_Siedlce_LD_2v2", "Siedlce #2v2")
CONFIG_OPTIONS(Map, "_3x2_Slutsk_LD_2v2", "Slutsk #2v2")
CONFIG_OPTIONS(Map, "_3x2_Slutsk_LD_3v3", "Slutsk #3v3")
CONFIG_OPTIONS(Map, "_3x2_Tali_Ihantala_LD_1v1", "Tali Ihantala #1v1")
CONFIG_OPTIONS(Map, "_3x2_Tali_Ihantala_LD_2v2", "Tali Ihantala #2v2")
CONFIG_OPTIONS(Map, "_3x2_Tali_Ihantala_LD_3v3", "Tali Ihantala #3v3")
CONFIG_OPTIONS(Map, "_3x2_Urban_roads_Krupki_LD_10v10", "Krupki #10vs10")
CONFIG_OPTIONS(Map, "_3x2_Urban_roads_Krupki_LD_3v3", "Krupki #3v3")
CONFIG_OPTIONS(Map, "_3x2_Veselovo_LD_2v2", "Veselovo #2v2")
CONFIG_OPTIONS(Map, "_3x2_West_Bobrujsk_LD_2v2", "Bobrujsk, west #2v2")
CONFIG_OPTIONS(Map, "_3x2_West_Brest_LD_1v1", "Brest, west #1v1")
CONFIG_OPTIONS(Map, "_3x2_West_Brest_LD_2v2", "Brest, west #2v2")
CONFIG_OPTIONS(Map, "_3x2_West_Brest_LD_3v3", "Brest, west #3v3")
CONFIG_OPTIONS(Map, "_3x2_Zbuczyn_LD_1v1", "Zbuczyn #1v1")
CONFIG_OPTIONS(Map, "_3x2_Zbuczyn_LD_2v2", "Zbuczyn #2v2")
CONFIG_OPTIONS(Map, "_4x2_Lenina_LD_4v4", "Lenina #4v4")
CONFIG_OPTIONS(Map, "_4x2_Ostrowno_LD_4v4", "Ostrowno #4v4")
CONFIG_OPTIONS(Map, "_4x2_Shchedrin_LD_4v4", "Shchedrin #4v4")
CONFIG_OPTIONS(Map, "_4x2_Slutsk_LD_10v10", "Slutsk DEST #10vs10")
CONFIG_OPTIONS(Map, "_4x2_Slutsk_LD_4v4", "Slutsk #4v4")
CONFIG_OPTIONS(Map, "_4x2_Vistula_Gora_Kalwaria_LD_1v1", "Gora Kalwaria #1v1")
CONFIG_OPTIONS(Map, "_4x2_Vistula_Gora_Kalwaria_LD_4v4", "Gora Kalwaria #4v4")
CONFIG_OPTIONS(Map, "_4x2_tannenberg_10vs10", "Tannenberg #10vs10")
CONFIG_OPTIONS(Map, "_4x2_Tannenberg_LD_1v1", "Tannenberg #1v1")
CONFIG_OPTIONS(Map, "_4x2_Tannenberg_LD_2v2", "Tannenberg #2v2")
CONFIG_OPTIONS(Map, "_2x3_Valley_Of_Death_LD_3v3", "Valley of Death #2v2")
CONFIG_OPTIONS(Map, "_4x2_Dukla_Pass_LD_4v4", "Dukla Pass #4v4")

#ifndef MAP_TAG
#define MAP_TAG(M, CONQUEST, BREAKTHROUGH, CLOSEQUARTER, DESTRUCTION, SMALL, MEDIUM, LARGE, VERYLARGE)
#endif

MAP_TAG("_2x1_Proto_levelBuild_Orsha_N_LD_1v1", 1, 1, 1, 1, 1, 0, 0, 0);
MAP_TAG("_2x2_Foret_Tsel_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_Kostritsa_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_Lacs_Sianno_LD_1v1", 1, 1, 1, 1, 1, 0, 0, 0);
MAP_TAG("_2x2_Lenina_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_Ostrowno_LD_1v1", 1, 1, 1, 1, 1, 0, 0, 0);
MAP_TAG("_2x2_Plateau_Central_Orsha_E_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_River_Swamp_Krupa_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_Shchedrin_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_Slutsk_E_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_Slutsk_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_Slutsk_W_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_Urban_River_Bobr_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x2_Ville_Centrale_Haroshaje_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_3x2_Astrouna_Novka_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Beshankovichy_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Bridges_Smolyany_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_East_Vitebsk_LD_2v2", 1, 1, 1, 1, 0, 1, 0, 0);
MAP_TAG("_3x2_Highway_LD", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Ilomantsi_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_3x2_Ilomantsi_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Island_Losnica_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Lenina_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Lenina_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Lipen_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Lyakhavichy_LD_3v3", 1, 1, 1, 1, 0, 0, 1, 0);
MAP_TAG("_3x2_Marecages_Naratch_lake_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Mogilev_LD_10v10", 1, 0, 0, 0, 0, 0, 0, 1);
MAP_TAG("_3x2_Mogilev_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Ostrowno_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Ostrowno_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Rivers_Pleshchenitsy_S_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Shchedrin_LD_2v2", 1, 1, 1, 1, 0, 1, 0, 0);
MAP_TAG("_3x2_Shchedrin_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Siedlce_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_3x2_Siedlce_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Slutsk_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Slutsk_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Tali_Ihantala_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_3x2_Tali_Ihantala_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_Tali_Ihantala_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Urban_roads_Krupki_LD_10v10", 1, 0, 0, 0, 0, 0, 0, 1);
MAP_TAG("_3x2_Urban_roads_Krupki_LD_3v3", 1, 1, 1, 0, 0, 0, 1, 0);
MAP_TAG("_3x2_Veselovo_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_West_Bobrujsk_LD_2v2", 1, 1, 1, 1, 0, 1, 0, 0);
MAP_TAG("_3x2_West_Brest_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_3x2_West_Brest_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_3x2_West_Brest_LD_3v3", 1, 1, 1, 1, 0, 0, 1, 0);
MAP_TAG("_3x2_Zbuczyn_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_3x2_Zbuczyn_LD_2v2", 1, 1, 1, 0, 0, 1, 0, 0);
MAP_TAG("_4x2_Lenina_LD_4v4", 1, 1, 1, 0, 0, 0, 0, 1);
MAP_TAG("_4x2_Ostrowno_LD_4v4", 1, 1, 1, 0, 0, 0, 0, 1);
MAP_TAG("_4x2_Shchedrin_LD_4v4", 1, 1, 1, 0, 0, 0, 0, 1);
MAP_TAG("_4x2_Slutsk_LD_10v10", 0, 0, 0, 1, 0, 0, 0, 1);
MAP_TAG("_4x2_Slutsk_LD_4v4", 1, 1, 1, 1, 0, 0, 0, 1);
MAP_TAG("_4x2_Vistula_Gora_Kalwaria_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_4x2_Vistula_Gora_Kalwaria_LD_4v4", 1, 1, 1, 1, 0, 0, 0, 1);
MAP_TAG("_4x2_tannenberg_10vs10", 1, 0, 0, 0, 0, 0, 0, 1);
MAP_TAG("_4x2_Tannenberg_LD_1v1", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_4x2_Tannenberg_LD_2v2", 1, 1, 1, 0, 1, 0, 0, 0);
MAP_TAG("_2x3_Valley_Of_Death_LD_3v3", 1, 1, 1, 1, 0, 0, 1, 0);
MAP_TAG("_4x2_Dukla_Pass_LD_4v4", 1, 1, 1, 1, 0, 0, 0, 1);

