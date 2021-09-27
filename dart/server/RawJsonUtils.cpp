#include "dart/server/RawJsonUtils.hpp"

namespace dart {

//==============================================================================
std::string escapeJson(const std::string& str)
{
  // TODO
  return str;
}

//==============================================================================
void vec2iToJson(std::stringstream& json, const Eigen::Vector2i& vec)
{
  json << "[" << vec(0) << "," << vec(1) << "]";
}

//==============================================================================
void vec2dToJson(std::stringstream& json, const Eigen::Vector2s& vec)
{
  json << "[" << vec(0) << "," << vec(1) << "]";
}

//==============================================================================
void vec3ToJson(std::stringstream& json, const Eigen::Vector3s& vec)
{
  json << "[" << vec(0) << "," << vec(1) << "," << vec(2) << "]";
}

//==============================================================================
void vec3iToJson(std::stringstream& json, const Eigen::Vector3i& vec)
{
  json << "[" << vec(0) << "," << vec(1) << "," << vec(2) << "]";
}

//==============================================================================
void vecXToJson(std::stringstream& json, const Eigen::VectorXs& vec)
{
  json << "[";
  for (int i = 0; i < vec.size(); i++)
  {
    json << vec(i);
    if (i < vec.size() - 1)
      json << ",";
  }
  json << "]";
}

//==============================================================================
void vecToJson(std::stringstream& json, const std::vector<s_t>& vec)
{
  json << "[";
  for (int i = 0; i < vec.size(); i++)
  {
    if (isfinite(vec[i]))
    {
      json << vec[i];
    }
    else
    {
      json << "0.0";
    }
    if (i < vec.size() - 1)
      json << ",";
  }
  json << "]";
}

} // namespace dart