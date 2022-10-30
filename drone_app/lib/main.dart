import 'dart:convert';
import 'dart:developer';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

import './models/drone-feedback.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // Try running your application with "flutter run". You'll see the
        // application has a blue toolbar. Then, without quitting the app, try
        // changing the primarySwatch below to Colors.green and then invoke
        // "hot reload" (press "r" in the console where you ran "flutter run",
        // or simply save your changes to "hot reload" in a Flutter IDE).
        // Notice that the counter didn't reset back to zero; the application
        // is not restarted.
        primarySwatch: Colors.blue,
      ),
      home: MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key? key, required this.title}) : super(key: key);

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  DroneFeedback? feedback;
  double speed = 0;

  var channel = WebSocketChannel.connect(
    //Uri.parse('wss://echo.websocket.org'),
    Uri.parse('ws://192.168.15.101/ws'),
  );

  @override
  void initState() {
    this.channel.stream.listen((data) {
      //log(data);
      var parsed = jsonDecode(data);
      //log("parse ok");
      var fb = DroneFeedback.fromJSON(parsed);
      this.setState(() {
        this.feedback = fb;
      });
    }, onDone: () {
      this.channel = WebSocketChannel.connect(
        //Uri.parse('wss://echo.websocket.org'),
        Uri.parse('ws://192.168.15.101/ws'),
      );
    }, onError: (obj) {
      this.channel = WebSocketChannel.connect(
        //Uri.parse('wss://echo.websocket.org'),
        Uri.parse('ws://192.168.15.101/ws'),
      );
    });
  }

  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.
    return Scaffold(
      appBar: AppBar(
        // Here we take the value from the MyHomePage object that was created by
        // the App.build method, and use it to set our appbar title.
        title: Text(widget.title),
      ),
      body: Column(
        children: [
          Text(speed.round().toString()),
          Slider(
            value: speed,
            max: 2500,
            min: 0,
            label: speed.round().toString(),
            onChanged: (double value) {
              setState(() {
                speed = value;
              });
            },
          ),
          ElevatedButton(
            child: Text("Atualizar Velocidade"),
            onPressed: () {
              channel.sink.add('{ "type": 2, "speed": ${speed.round()}}');
            },
          ),
          ...renderFeedBack()
        ],
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: _sendMessage,
        tooltip: 'Send message',  
        child: Icon(Icons.send),
      ),
    );
  }

  void _sendMessage() {
    channel.sink.add('{ "type": 1, "message": "teste bem sucedido"}');
  }

  List<Widget> renderFeedBack() {
    if (this.feedback == null) {
      return [Text("Waiting for data")];
    }

    return [
      renderFeedBackItem("quaternion x: ", this.feedback?.quaternionX?.toStringAsFixed(4) ?? "0"),
      renderFeedBackItem("quaternion y: ", this.feedback?.quaternionX?.toStringAsFixed(4) ?? "0"),
      renderFeedBackItem("quaternion z: ", this.feedback?.quaternionX?.toStringAsFixed(4) ?? "0"),
      renderFeedBackItem("quaternion w: ", this.feedback?.quaternionX?.toStringAsFixed(4) ?? "0"),
      SizedBox(height: 20),
      renderFeedBackItem("euler x: ", this.feedback?.eulerX?.toStringAsFixed(4) ?? "0"),
      renderFeedBackItem("euler y: ", this.feedback?.eulerY?.toStringAsFixed(4) ?? "0"),
      renderFeedBackItem("euler z: ", this.feedback?.eulerZ?.toStringAsFixed(4) ?? "0"),
      SizedBox(height: 20),
      renderFeedBackItem("rool: ", this.feedback?.roll?.toStringAsFixed(4) ?? "0"),
      renderFeedBackItem("pitch: ", this.feedback?.pitch?.toStringAsFixed(4) ?? "0"),
      renderFeedBackItem("yaw: ", this.feedback?.yaw?.toStringAsFixed(4) ?? "0"),
    ];
  }

  renderFeedBackItem(String text, String value) {
    return Row(
      children: [
        Text(text),
        Text(value),
      ],
    );
  }
}
