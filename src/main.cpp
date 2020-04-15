#include <chrono>
#include <string>
#include <memory>
#include <filesystem>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/clock.hpp"
#include "std_msgs/msg/float64.hpp"
#include "ros2_idl_example/srv/add_floats.hpp"

using namespace std::chrono_literals;

class APublisher : public rclcpp::Node
{
public:
  APublisher()
  : Node("theNode")
  {
    const auto qos_history_depth = 10;
    publisher_last_result_ =
      this->create_publisher<std_msgs::msg::Float64>(
      std::string(
        get_name()) + "/value/lastresult", qos_history_depth);
  }

  void set_last_value(float c)
  {
    last_result_.data = c;
  }

  void publish_values()
  {
    publisher_last_result_->publish(last_result_);
  }

private:
  std_msgs::msg::Float64 last_result_;

  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr publisher_last_result_;
};

std::shared_ptr<APublisher> publisher;

void add_floats(
  const std::shared_ptr<ros2_idl_example::srv::AddFloats::Request> request,
  std::shared_ptr<ros2_idl_example::srv::AddFloats::Response> response)
{
  RCLCPP_INFO(
    rclcpp::get_logger("rclcpp"), "a %f, b %f",
    request->a, request->b);

  response->c = request->a + request->b;

  publisher->set_last_value(response->c);
  publisher->publish_values();
}


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  publisher = std::make_shared<APublisher>();

  std::shared_ptr<rclcpp::Node> service_node = rclcpp::Node::make_shared("service_example");
  rclcpp::Service<ros2_idl_example::srv::AddFloats>::SharedPtr service =
    service_node->create_service<ros2_idl_example::srv::AddFloats>(
    "service/test_message", &add_floats);

  auto appname = std::filesystem::path(argv[0]).filename().string();

  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(publisher);
  executor.add_node(service_node);

  std::cout << appname << " starting" << std::endl;
  executor.spin();

  rclcpp::shutdown();
  std::cout << appname << " shutting down" << std::endl;
  return 0;
}
