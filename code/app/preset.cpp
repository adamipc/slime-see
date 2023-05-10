#include "app/preset.h"

function u32
random_range(u32 min, u32 max) {
  if (min == max) return min;
  if (min > max) {
    u32 temp = min;
    min = max;
    max = temp;
  }
  u32 result = min + (rand() % (max - min));
  return result;
}

function Preset lerp_preset(Preset a, Preset b, f32 t) {
  Preset result = {};

  result.number_of_points = lerp_i32(a.number_of_points, t, b.number_of_points);
  result.starting_arrangement = (StartingArrangement)lerp_i32(a.starting_arrangement, t, b.starting_arrangement);
  result.average_starting_speed = lerp_f32(a.average_starting_speed, t, b.average_starting_speed);
  result.starting_speed_spread = lerp_f32(a.starting_speed_spread, t, b.starting_speed_spread);
  
  result.speed_multiplier = lerp_f32(a.speed_multiplier, t, b.speed_multiplier);
  result.point_size = lerp_f32(a.point_size, t, b.point_size);
  result.random_steer_factor = lerp_f32(a.random_steer_factor, t, b.random_steer_factor);
  result.constant_steer_factor = lerp_f32(a.constant_steer_factor, t, b.constant_steer_factor);
  result.trail_strength = lerp_f32(a.trail_strength, t, b.trail_strength);
  result.search_radius = lerp_f32(a.search_radius, t, b.search_radius);
  result.wall_strategy = (WallStrategy)lerp_f32(a.wall_strategy, t, b.wall_strategy);
  result.color_strategy = (ColorStrategy)lerp_f32(a.color_strategy, t, b.color_strategy);

  result.fade_speed = lerp_f32(a.fade_speed, t, b.fade_speed);
  result.blurring = lerp_f32(a.blurring, t, b.blurring);

  return result;
}

function Preset
randomize_preset() {
  Preset result = {};

  result.number_of_points = power_of_two(random_range(12, 21)); // 2048 - 2097152
  result.starting_arrangement = (StartingArrangement)random_range(0, StartingArrangement_COUNT);

  result.average_starting_speed = random_range(0, 100) / 100.0f; // 0.0 - 1.0
  result.starting_speed_spread = random_range(0, 100) / 100.0f;  // 0.0 - 1.0

  // TODO(adam): Do we want to randomize this? Currently all presets use the same value
  result.speed_multiplier = 1.0f;
  result.point_size = (random_range(0, 10) / 2.5f) + 1.0f;       // 1.0 - 5.0
  result.random_steer_factor = random_range(5, 100) / 1000.0f;   // 0.005 - 0.1
  result.constant_steer_factor = random_range(0, 500) / 1000.0f; // 0.0 - 0.5
  result.trail_strength = random_range(0, 200) / 1000.0f;        // 0.0 - 0.2
  result.search_radius = random_range(10, 100) / 1000.0f;        // 0.01 - 0.1
  result.wall_strategy = (WallStrategy)random_range(1, WallStrategy_COUNT);
  result.color_strategy = (ColorStrategy)random_range(0, ColorStrategy_COUNT);

  result.fade_speed = random_range(0, 100) / 1000.0f; // 0.0 - 0.1
  result.blurring = random_range(10, 100) / 100.0f;  // 0.1 - 1.0

  return result;
}

function Preset
get_preset(PresetNames preset_name) {
  Preset result = {};

  switch (preset_name) {
    case PresetName_GreenSlime: {
      result.number_of_points = power_of_two(18);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 0.0f;
      result.starting_speed_spread = 0.3f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.1f;
      result.constant_steer_factor = 0.1f;
      result.trail_strength = 0.01f;
      result.search_radius = 0.01f;
      result.wall_strategy = WallStrategy_Bounce;
      result.color_strategy = ColorStrategy_Position;

      result.fade_speed = 0.01f;
      result.blurring = 1.0f;
    } break;
    case PresetName_CollapsingBubble: {
      result.number_of_points = power_of_two(11);
      result.starting_arrangement = StartingArrangement_Ring;
      result.average_starting_speed = 0.5f;
      result.starting_speed_spread = 0.1f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.5f;
      result.random_steer_factor = 0.1f;
      result.constant_steer_factor = 0.5f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.005f;
      result.blurring = 1.0f;
    } break;
    case PresetName_SlimeRing: {
      result.number_of_points = power_of_two(18);
      result.starting_arrangement = StartingArrangement_Ring;
      result.average_starting_speed = 0.1f;
      result.starting_speed_spread = 0.1f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.1f;
      result.constant_steer_factor = 0.4f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.01f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Grey;

      result.fade_speed = 0.05f;
      result.blurring = 1.0f;
    } break;
    case PresetName_ShiftingWeb: {
      result.number_of_points = power_of_two(16);
      result.starting_arrangement = StartingArrangement_Ring;
      result.average_starting_speed = 1.0f;
      result.starting_speed_spread = 0.1f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.1f;
      result.constant_steer_factor = 0.45f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.05f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Position;

      result.fade_speed = 0.07f;
      result.blurring = 1.0f;
    } break;
    case PresetName_Waves: {
      result.number_of_points = power_of_two(16);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 1.0f;
      result.starting_speed_spread = 0.0f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.04f;
      result.constant_steer_factor = 0.07f;
      result.trail_strength = 0.1f;
      result.search_radius = 0.01f;
      result.wall_strategy = WallStrategy_Bounce;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.04f;
      result.blurring = 1.0f;
    } break;
    case PresetName_Flower: {
      result.number_of_points = power_of_two(15);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 0.0f;
      result.starting_speed_spread = 0.8f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.02f;
      result.constant_steer_factor = 0.04f;
      result.trail_strength = 0.5f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_Bounce;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.02f;
      result.blurring = 1.0f;
    } break;
    case PresetName_ChristmasChaos: {
      result.number_of_points = power_of_two(12);
      result.starting_arrangement = StartingArrangement_Random;
      result.average_starting_speed = 0.9f;
      result.starting_speed_spread = 0.0f;

      result.speed_multiplier = 1.0f;
      result.point_size = 3.0f;
      result.random_steer_factor = 0.10f;
      result.constant_steer_factor = 4.00f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.02f;
      result.blurring = 1.0f;
    } break;
    case PresetName_Explode: {
      result.number_of_points = power_of_two(18);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 0.4f;
      result.starting_speed_spread = 0.3f;

      result.speed_multiplier = 1.0f;
      result.point_size = 2.0f;
      result.random_steer_factor = 0.05f;
      result.constant_steer_factor = 0.10f;
      result.trail_strength = 0.2f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_None;
      result.color_strategy = ColorStrategy_Grey;

      result.fade_speed = 0.00f;
      result.blurring = 0.0f;
    } break;
    case PresetName_Tartan: {
      result.number_of_points = power_of_two(16);
      result.starting_arrangement = StartingArrangement_Origin;
      result.average_starting_speed = 0.8f;
      result.starting_speed_spread = 0.1f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.05f;
      result.constant_steer_factor = 0.01f;
      result.trail_strength = 0.01f;
      result.search_radius = 0.1f;
      result.wall_strategy = WallStrategy_Wrap;
      result.color_strategy = ColorStrategy_Direction;

      result.fade_speed = 0.01f;
      result.blurring = 1.0f;
    } break;
    case PresetName_Globe: {
      result.number_of_points = power_of_two(16);
      result.starting_arrangement = StartingArrangement_Ring;
      result.average_starting_speed = 0.0f;
      result.starting_speed_spread = 0.3f;

      result.speed_multiplier = 1.0f;
      result.point_size = 1.0f;
      result.random_steer_factor = 0.005f;
      result.constant_steer_factor = 0.00f;
      result.trail_strength = 0.20f;
      result.search_radius = 0.01f;
      result.wall_strategy = WallStrategy_Bounce;
      result.color_strategy = ColorStrategy_ShiftingHue;

      result.fade_speed = 0.117f;
      result.blurring = 1.0f;
    } break;
  }

  return result;
}

