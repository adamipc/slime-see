#ifndef PRESET_H
#define PRESET_H

typedef u8 StartingArrangement;
enum{
  StartingArrangement_Ring,
  StartingArrangement_Random,
  StartingArrangement_Origin,
  StartingArrangement_COUNT,
};

typedef u8 WallStrategy;
enum{
  WallStrategy_None = 0,
  WallStrategy_Wrap = 1,
  WallStrategy_Bounce = 2,
  WallStrategy_BounceRandom = 3,
  WallStrategy_SlowAndReverse = 4,
  WallStrategy_COUNT = 5,
};

typedef u8 ColorStrategy;
enum{
  ColorStrategy_Direction = 0,
  ColorStrategy_Speed = 1,
  ColorStrategy_Position = 2,
  ColorStrategy_Grey = 3,
  ColorStrategy_ShiftingHue = 4,
  ColorStrategy_Distance = 5,
  ColorStrategy_Oscillation = 6,
  ColorStrategy_COUNT,
};

struct Preset {
  u32                   number_of_points;
  StartingArrangement   starting_arrangement;
  float                 average_starting_speed;
  float                 starting_speed_spread;

  float                 speed_multiplier;
  float                 point_size;
  float                 random_steer_factor;
  float                 constant_steer_factor;
  float                 trail_strength;
  float                 search_radius;
  WallStrategy          wall_strategy;
  ColorStrategy         color_strategy;

  float                 fade_speed;
  float                 blurring;
};

typedef u8 PresetNames;
enum{
  PresetName_None = 0,

  PresetName_GreenSlime,
  PresetName_CollapsingBubble,
  PresetName_SlimeRing,
  PresetName_ShiftingWeb,
  PresetName_Waves,
  PresetName_Flower,
  PresetName_ChristmasChaos,
  PresetName_Explode,
  PresetName_Tartan,
  PresetName_Globe,

  PresetName_COUNT,
};

function Preset randomize_preset();
function Preset lerp_preset(Preset a, Preset b, f32 t);

#endif // PRESET_H
