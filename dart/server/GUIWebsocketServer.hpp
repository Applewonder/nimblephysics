#ifndef DART_GUI_SERVER
#define DART_GUI_SERVER

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Eigen/Dense>
#include <asio/io_service.hpp>
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include "dart/dynamics/BodyNode.hpp"
#include "dart/dynamics/MeshShape.hpp"
#include "dart/server/WebsocketServer.hpp"

namespace dart {

namespace simulation {
class World;
}

namespace dynamics {
class Skeleton;
}

namespace trajectory {
class TrajectoryRollout;
}

namespace server {

class GUIWebsocketServer
{
public:
  GUIWebsocketServer();

  ~GUIWebsocketServer();

  /// This is a non-blocking call to start a websocket server on a given
  /// port
  void serve(int port);

  /// This kills the server, if one was running
  void stopServing();

  /// Returns true if we're serving
  bool isServing();

  /// This sleeps until we're done serving, without busy-waiting in a loop. It
  /// wakes up occassionally to call the `checkForSignals` callback, where you
  /// can throw an exception to shut down the program.
  void blockWhileServing(std::function<void()> checkForSignals = []() {});

  /// This adds a listener that will get called when someone connects to the
  /// server
  void registerConnectionListener(std::function<void()> listener);

  /// This adds a listener that will get called when ctrl+C is pressed
  void registerShutdownListener(std::function<void()> listener);

  /// This adds a listener that will get called when there is a key-down
  /// event on the web client
  void registerKeydownListener(std::function<void(std::string)> listener);

  /// This adds a listener that will get called when there is a key-up event
  /// on the web client
  void registerKeyupListener(std::function<void(std::string)> listener);

  /// Gets the set of all the keys currently being pressed
  const std::unordered_set<std::string>& getKeysDown() const;

  /// Returns true if a key is currently being pressed
  bool isKeyDown(const std::string& key) const;

  /// This tells us whether or not to automatically flush after each command
  void setAutoflush(bool autoflush);

  /// This tells us whether or not to automatically flush after each command
  bool getAutoflush();

  /// This sends the current list of commands to the web GUI
  void flush();

  /// This is a high-level command that creates/updates all the shapes in a
  /// world by calling the lower-level commands
  GUIWebsocketServer& renderWorld(
      const std::shared_ptr<simulation::World>& world,
      const std::string& prefix = "world",
      bool renderForces = true,
      bool renderForceMagnitudes = true);

  /// This is a high-level command that creates a basis
  GUIWebsocketServer& renderBasis(
      s_t scale = 10.0,
      const std::string& prefix = "basis",
      const Eigen::Vector3s pos = Eigen::Vector3s::Zero(),
      const Eigen::Vector3s euler = Eigen::Vector3s::Zero());

  /// This is a high-level command that creates/updates all the shapes in a
  /// world by calling the lower-level commands
  GUIWebsocketServer& renderSkeleton(
      const std::shared_ptr<dynamics::Skeleton>& skel,
      const std::string& prefix = "skel",
      Eigen::Vector3s overrideColor = -1 * Eigen::Vector3s::Ones());

  /// This is a high-level command that renders a given trajectory as a
  /// bunch of lines in the world, one per body
  GUIWebsocketServer& renderTrajectoryLines(
      std::shared_ptr<simulation::World> world,
      Eigen::MatrixXs positions,
      std::string prefix = "trajectory");

  /// This is a high-level command that renders a wrench on a body node
  GUIWebsocketServer& renderBodyWrench(
      const dynamics::BodyNode* body,
      Eigen::Vector6s wrench,
      s_t scaleFactor = 0.1,
      std::string prefix = "wrench");

  /// This renders little velocity lines starting at every vertex in the passed
  /// in body
  GUIWebsocketServer& renderMovingBodyNodeVertices(
      const dynamics::BodyNode* body,
      s_t scaleFactor = 0.1,
      std::string prefix = "vert-vel");

  /// This is a high-level command that removes the lines rendering a wrench on
  /// a body node
  GUIWebsocketServer& clearBodyWrench(
      const dynamics::BodyNode* body, std::string prefix = "wrench");

  /// This completely resets the web GUI, deleting all objects, UI elements,
  /// and listeners
  GUIWebsocketServer& clear();

  /// This creates a box in the web GUI under a specified key
  GUIWebsocketServer& createBox(
      std::string key,
      const Eigen::Vector3s& size,
      const Eigen::Vector3s& pos,
      const Eigen::Vector3s& euler,
      const Eigen::Vector3s& color = Eigen::Vector3s(0.5, 0.5, 0.5),
      bool castShadows = false,
      bool receiveShadows = false);

  /// This creates a sphere in the web GUI under a specified key
  GUIWebsocketServer& createSphere(
      std::string key,
      s_t radius,
      const Eigen::Vector3s& pos,
      const Eigen::Vector3s& color = Eigen::Vector3s(0.5, 0.5, 0.5),
      bool castShadows = false,
      bool receiveShadows = false);

  /// This creates a capsule in the web GUI under a specified key
  GUIWebsocketServer& createCapsule(
      std::string key,
      s_t radius,
      s_t height,
      const Eigen::Vector3s& pos,
      const Eigen::Vector3s& euler,
      const Eigen::Vector3s& color = Eigen::Vector3s(0.5, 0.5, 0.5),
      bool castShadows = false,
      bool receiveShadows = false);

  /// This creates a line in the web GUI under a specified key
  GUIWebsocketServer& createLine(
      std::string key,
      const std::vector<Eigen::Vector3s>& points,
      const Eigen::Vector3s& color = Eigen::Vector3s(1.0, 0.5, 0.5));

  /// This creates a mesh in the web GUI under a specified key, using raw shape
  /// data
  GUIWebsocketServer& createMesh(
      std::string key,
      const std::vector<Eigen::Vector3s>& vertices,
      const std::vector<Eigen::Vector3s>& vertexNormals,
      const std::vector<Eigen::Vector3i>& faces,
      const std::vector<Eigen::Vector2s>& uv,
      const std::vector<std::string>& textures,
      const std::vector<int>& textureStartIndices,
      const Eigen::Vector3s& pos,
      const Eigen::Vector3s& euler,
      const Eigen::Vector3s& scale = Eigen::Vector3s::Ones(),
      const Eigen::Vector3s& color = Eigen::Vector3s(0.5, 0.5, 0.5),
      bool castShadows = false,
      bool receiveShadows = false);

  /// This creates a mesh in the web GUI under a specified key, from the ASSIMP
  /// mesh
  GUIWebsocketServer& createMeshASSIMP(
      const std::string& key,
      const aiScene* mesh,
      const std::string& meshPath,
      const Eigen::Vector3s& pos,
      const Eigen::Vector3s& euler,
      const Eigen::Vector3s& scale = Eigen::Vector3s::Ones(),
      const Eigen::Vector3s& color = Eigen::Vector3s(0.5, 0.5, 0.5),
      bool castShadows = false,
      bool receiveShadows = false);

  /// This creates a mesh in the web GUI under a specified key, from the ASSIMP
  /// mesh
  GUIWebsocketServer& createMeshFromShape(
      const std::string& key,
      const std::shared_ptr<dynamics::MeshShape> mesh,
      const Eigen::Vector3s& pos,
      const Eigen::Vector3s& euler,
      const Eigen::Vector3s& scale = Eigen::Vector3s::Ones(),
      const Eigen::Vector3s& color = Eigen::Vector3s(0.5, 0.5, 0.5),
      bool castShadows = false,
      bool receiveShadows = false);

  /// This creates a texture object, to be sent to the web frontend
  GUIWebsocketServer& createTexture(
      const std::string& key, const std::string& base64);

  /// This creates a texture object by loading it from a file
  GUIWebsocketServer& createTextureFromFile(
      const std::string& key, const std::string& path);

  /// This returns true if we've already got an object with the key "key"
  bool hasObject(const std::string& key);

  /// This returns the position of an object, if we've got it (and it's not
  /// a line). Otherwise it returns Vector3s::Zero().
  Eigen::Vector3s getObjectPosition(const std::string& key);

  /// This returns the rotation of an object, if we've got it (and it's not
  /// a line or a sphere). Otherwise it returns Vector3s::Zero().
  Eigen::Vector3s getObjectRotation(const std::string& key);

  /// This returns the color of an object, if we've got it. Otherwise it
  /// returns Vector3s::Zero().
  Eigen::Vector3s getObjectColor(const std::string& key);

  /// This returns the size of a box, scale of a mesh, 3vec of [radius, radius,
  /// radius] for a sphere, and [radius, radius, height] for a capsule. Returns
  /// 0 for lines.
  Eigen::Vector3s getObjectScale(const std::string& key);

  /// This moves an object (e.g. box, sphere, line) to a specified position
  GUIWebsocketServer& setObjectPosition(
      const std::string& key, const Eigen::Vector3s& pos);

  /// This moves an object (e.g. box, sphere, line) to a specified
  /// orientation
  GUIWebsocketServer& setObjectRotation(
      const std::string& key, const Eigen::Vector3s& euler);

  /// This changes an object (e.g. box, sphere, line) color
  GUIWebsocketServer& setObjectColor(
      const std::string& key, const Eigen::Vector3s& color);

  /// This changes an object (e.g. box, sphere, mesh) size. Has no effect on
  /// lines.
  GUIWebsocketServer& setObjectScale(
      const std::string& key, const Eigen::Vector3s& scale);

  /// This enables mouse events on an object (if they're not already), and
  /// calls "listener" whenever the object is dragged with the desired drag
  /// coordinates
  GUIWebsocketServer& registerDragListener(
      const std::string& key, std::function<void(Eigen::Vector3s)> listener);

  /// This deletes an object by key
  GUIWebsocketServer& deleteObject(const std::string& key);

  /// This deletes all the objects that match a given prefix
  GUIWebsocketServer& deleteObjectsByPrefix(const std::string& prefix);

  /// This gets the current screen size
  Eigen::Vector2i getScreenSize();

  /// This registers a callback to get called whenever the screen size changes.
  void registerScreenResizeListener(
      std::function<void(Eigen::Vector2i)> listener);

  /// This places some text on the screen at the specified coordinates
  GUIWebsocketServer& createText(
      const std::string& key,
      const std::string& contents,
      const Eigen::Vector2i& fromTopLeft,
      const Eigen::Vector2i& size);

  /// This changes the contents of text on the screen
  GUIWebsocketServer& setTextContents(
      const std::string& key, const std::string& newContents);

  /// This places a clickable button on the screen at the specified
  /// coordinates
  GUIWebsocketServer& createButton(
      const std::string& key,
      const std::string& label,
      const Eigen::Vector2i& fromTopLeft,
      const Eigen::Vector2i& size,
      std::function<void()> onClick);

  /// This changes the contents of text on the screen
  GUIWebsocketServer& setButtonLabel(
      const std::string& key, const std::string& newLabel);

  /// This creates a slider
  GUIWebsocketServer& createSlider(
      const std::string& key,
      const Eigen::Vector2i& fromTopLeft,
      const Eigen::Vector2i& size,
      s_t min,
      s_t max,
      s_t value,
      bool onlyInts,
      bool horizontal,
      std::function<void(s_t)> onChange);

  /// This changes the contents of text on the screen
  GUIWebsocketServer& setSliderValue(const std::string& key, s_t value);

  /// This changes the contents of text on the screen
  GUIWebsocketServer& setSliderMin(const std::string& key, s_t min);

  /// This changes the contents of text on the screen
  GUIWebsocketServer& setSliderMax(const std::string& key, s_t max);

  /// This creates a plot to display data on the GUI
  GUIWebsocketServer& createPlot(
      const std::string& key,
      const Eigen::Vector2i& fromTopLeft,
      const Eigen::Vector2i& size,
      const std::vector<s_t>& xs,
      s_t minX,
      s_t maxX,
      const std::vector<s_t>& ys,
      s_t minY,
      s_t maxY,
      const std::string& type);

  /// This changes the contents of a plot, along with its display limits
  GUIWebsocketServer& setPlotData(
      const std::string& key,
      const std::vector<s_t>& xs,
      s_t minX,
      s_t maxX,
      const std::vector<s_t>& ys,
      s_t minY,
      s_t maxY);

  /// This moves a UI element on the screen
  GUIWebsocketServer& setUIElementPosition(
      const std::string& key, const Eigen::Vector2i& fromTopLeft);

  /// This changes the size of a UI element
  GUIWebsocketServer& setUIElementSize(
      const std::string& key, const Eigen::Vector2i& size);

  /// This deletes a UI element by key
  GUIWebsocketServer& deleteUIElement(const std::string& key);

protected:
  int mPort;
  bool mServing;
  bool mStartingServer;
  Eigen::Vector2i mScreenSize;
  asio::signal_set* mSignalSet;
  std::thread* mServerThread;
  WebsocketServer* mServer;
  std::recursive_mutex globalMutex;
  std::mutex mServingMutex;
  std::condition_variable mServingConditionValue;

  // protects the buffered JSON message (mJson) from getting
  // corrupted if we queue messages while trying to flush()
  std::recursive_mutex mJsonMutex;
  bool mAutoflush;
  int mMessagesQueued;
  std::stringstream mJson;

  // Listeners
  std::vector<std::function<void()>> mConnectionListeners;
  std::vector<std::function<void()>> mShutdownListeners;
  std::vector<std::function<void(std::string)>> mKeydownListeners;
  std::vector<std::function<void(std::string)>> mKeyupListeners;
  std::unordered_set<std::string> mKeysDown;
  std::unordered_map<
      std::string,
      std::vector<std::function<void(Eigen::Vector3s)>>>
      mDragListeners;
  std::vector<std::function<void(Eigen::Vector2i)>> mScreenResizeListeners;
  // This is a list of all the objects with mouse interaction enabled
  std::unordered_set<std::string> mMouseInteractionEnabled;

  struct Box
  {
    std::string key;
    Eigen::Vector3s size;
    Eigen::Vector3s pos;
    Eigen::Vector3s euler;
    Eigen::Vector3s color;
    bool castShadows;
    bool receiveShadows;
  };
  std::unordered_map<std::string, Box> mBoxes;
  struct Sphere
  {
    std::string key;
    s_t radius;
    Eigen::Vector3s pos;
    Eigen::Vector3s color;
    bool castShadows;
    bool receiveShadows;
  };
  std::unordered_map<std::string, Sphere> mSpheres;

  struct Capsule
  {
    std::string key;
    s_t radius;
    s_t height;
    Eigen::Vector3s pos;
    Eigen::Vector3s euler;
    Eigen::Vector3s color;
    bool castShadows;
    bool receiveShadows;
  };
  std::unordered_map<std::string, Capsule> mCapsules;
  struct Line
  {
    std::string key;
    std::vector<Eigen::Vector3s> points;
    Eigen::Vector3s color;
  };
  std::unordered_map<std::string, Line> mLines;

  struct Mesh
  {
    std::string key;
    std::vector<Eigen::Vector3s> vertices;
    std::vector<Eigen::Vector3s> vertexNormals;
    std::vector<Eigen::Vector3i> faces;
    std::vector<Eigen::Vector2s> uv;
    std::vector<std::string> textures;
    std::vector<int> textureStartIndices;
    Eigen::Vector3s pos;
    Eigen::Vector3s euler;
    Eigen::Vector3s scale;
    Eigen::Vector3s color;
    bool castShadows;
    bool receiveShadows;
  };
  std::unordered_map<std::string, Mesh> mMeshes;

  struct Texture
  {
    std::string key;
    std::string base64;
  };
  std::unordered_map<std::string, Texture> mTextures;

  struct Text
  {
    std::string key;
    std::string contents;
    Eigen::Vector2i fromTopLeft;
    Eigen::Vector2i size;
  };
  std::unordered_map<std::string, Text> mText;

  struct Button
  {
    std::string key;
    std::string label;
    Eigen::Vector2i fromTopLeft;
    Eigen::Vector2i size;
    std::function<void()> onClick;
  };
  std::unordered_map<std::string, Button> mButtons;

  struct Slider
  {
    std::string key;
    Eigen::Vector2i fromTopLeft;
    Eigen::Vector2i size;
    s_t min;
    s_t max;
    s_t value;
    bool onlyInts;
    bool horizontal;
    std::function<void(s_t)> onChange;
  };
  std::unordered_map<std::string, Slider> mSliders;

  struct Plot
  {
    std::string key;
    Eigen::Vector2i fromTopLeft;
    Eigen::Vector2i size;
    std::vector<s_t> xs;
    s_t minX;
    s_t maxX;
    std::vector<s_t> ys;
    s_t minY;
    s_t maxY;
    std::string type;
  };
  std::unordered_map<std::string, Plot> mPlots;

  void queueCommand(std::function<void(std::stringstream&)> writeCommand);

  void encodeCreateBox(std::stringstream& json, Box& box);
  void encodeCreateSphere(std::stringstream& json, Sphere& sphere);
  void encodeCreateCapsule(std::stringstream& json, Capsule& capsule);
  void encodeCreateLine(std::stringstream& json, Line& line);
  void encodeCreateMesh(std::stringstream& json, Mesh& mesh);
  void encodeCreateTexture(std::stringstream& json, Texture& texture);
  void encodeEnableMouseInteraction(
      std::stringstream& json, const std::string& key);
  void encodeCreateText(std::stringstream& json, Text& text);
  void encodeCreateButton(std::stringstream& json, Button& button);
  void encodeCreateSlider(std::stringstream& json, Slider& slider);
  void encodeCreatePlot(std::stringstream& json, Plot& plot);
};

} // namespace server
} // namespace dart

#endif