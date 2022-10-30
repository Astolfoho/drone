class DroneFeedback {
  // return Photo(
  //   albumId: json['albumId'] as int,
  //   id: json['id'] as int,
  //   title: json['title'] as String,
  //   url: json['url'] as String,
  //   thumbnailUrl: json['thumbnailUrl'] as String,

  DroneFeedback();

  DroneFeedback.fromJSON(Map<String, dynamic> json) {
    quaternionX = json['q_x'] as double;
    quaternionY = json['q_y'] as double;
    quaternionZ = json['q_z'] as double;
    quaternionW = json['q_w'] as double;

    eulerX = json['e_x'] as double;
    eulerY = json['e_y'] as double;
    eulerZ = json['e_z'] as double;

    roll = json['roll'] as double;
    pitch = json['pitch'] as double;
    yaw = json['yaw'] as double;
  }

  double? quaternionX;
  double? quaternionY;
  double? quaternionZ;
  double? quaternionW;

  double? eulerX;
  double? eulerY;
  double? eulerZ;

  double? roll;
  double? pitch;
  double? yaw;

  //   doc["q_x"] = quat.x;
  //   doc["q_y"] = quat.y;
  //   doc["q_z"] = quat.z;
  //   doc["q_w"] = quat.w;

  //   doc["e_x"] = euler.x;
  //   doc["e_y"] = euler.y;
  //   doc["e_z"] = euler.z;

  //   doc["roll"] = rpy.r;
  //   doc["pitch"] = rpy.p;
  //   doc["yaw"] = rpy.y;

}
