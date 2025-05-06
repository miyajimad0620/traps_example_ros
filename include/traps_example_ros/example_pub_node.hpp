// Copyright 2025 TRAPS
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TRAPS_EXAMPLE_ROS__EXAMPLE_PUB_NODE_HPP_
#define TRAPS_EXAMPLE_ROS__EXAMPLE_PUB_NODE_HPP_

#include <deque>
#include <string>
#include <utility>
#include <vector>

#include "fmt/format.h"
#include "rclcpp/node.hpp"
#include "std_msgs/msg/empty.hpp"
#include "traps_example_ros/visibility.hpp"

namespace traps_example_ros
{

class ExamplePubNode : public rclcpp::Node
{
public:
  static constexpr auto kDefaultNodeName = "traps_example";

  static constexpr auto kRateParamName = "rate";

  TRAPS_EXAMPLE_ROS_PUBLIC
  inline ExamplePubNode(
    const std::string & node_name, const std::string & node_namespace,
    const rclcpp::NodeOptions & node_options = rclcpp::NodeOptions())
  : rclcpp::Node(node_name, node_namespace, node_options)
  {
    // Create a pub/sub options
    static const auto qos_overriding_options = rclcpp::QosOverridingOptions{
      rclcpp::QosPolicyKind::Depth, rclcpp::QosPolicyKind::Durability,
      rclcpp::QosPolicyKind::History, rclcpp::QosPolicyKind::Reliability};
    static const auto pub_options = [&] {
        rclcpp::PublisherOptions options;
        options.qos_overriding_options = qos_overriding_options;
        return options;
      }();

    // create pub/sub
    pubsub_test_pub_ = this->create_publisher<std_msgs::msg::Empty>(
      "pubsub_test", rclcpp::QoS(1).best_effort(), pub_options);

    // set parameter callback
    param_handle_ =
      this->add_on_set_parameters_callback(
      [this](const std::vector<rclcpp::Parameter> & params) {
        return this->update_params(params);
      });

    // declare parameter
    this->declare_parameter(kRateParamName, 1.0);
  }

  TRAPS_EXAMPLE_ROS_PUBLIC
  explicit inline ExamplePubNode(
    const std::string & node_name, const rclcpp::NodeOptions & node_options = rclcpp::NodeOptions())
  : ExamplePubNode(node_name, "", node_options)
  {
  }

  TRAPS_EXAMPLE_ROS_PUBLIC
  explicit inline ExamplePubNode(const rclcpp::NodeOptions & node_options = rclcpp::NodeOptions())
  : ExamplePubNode(kDefaultNodeName, "", node_options)
  {
  }

private:
  void publish_pubsub_test()
  {
    auto pubsub_test_msg = std::make_unique<std_msgs::msg::Empty>();
    RCLCPP_INFO(this->get_logger(), "published address: %p", static_cast<void *>(pubsub_test_msg.get()));
    pubsub_test_pub_->publish(std::move(pubsub_test_msg));
  }

  rcl_interfaces::msg::SetParametersResult update_params(
    const std::vector<rclcpp::Parameter> & params)
  {
    // set parameters
    std::deque<std::string> error_strs;
    for (const auto & param : params) {
      // set each parameter
      if (param.get_name() == kRateParamName) {
        const auto rate = param.as_double();
        publish_timer_ = this->create_wall_timer(
          std::chrono::duration<double>(1.0 / rate),
          [this]() {
            this->publish_pubsub_test();
          });
      } else {
        error_strs.push_back(
          fmt::format("unknown parameter({}: {})", param.get_name(), param.value_to_string()));
      }
    }

    // print errors
    for (const auto & error_str : error_strs) {
      RCLCPP_ERROR(this->get_logger(), "Failed to set parameter: %s", error_str.c_str());
    }

    // return value
    rcl_interfaces::msg::SetParametersResult set_parameters_result_msg;
    set_parameters_result_msg.successful = error_strs.empty();
    set_parameters_result_msg.reason = fmt::format("{}", fmt::join(error_strs, "; "));
    return set_parameters_result_msg;
  }

  // publisher
  rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr pubsub_test_pub_;

  // publish timer
  rclcpp::TimerBase::SharedPtr publish_timer_;

  // parameter
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_handle_;
};

}  // namespace traps_example_ros

#endif  // TRAPS_EXAMPLE_ROS__EXAMPLE_PUB_NODE_HPP_
