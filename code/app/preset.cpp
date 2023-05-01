#include "app/preset.h"

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

