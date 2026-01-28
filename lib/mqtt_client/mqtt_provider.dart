import 'dart:async';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'mqtt_client_factory.dart';

enum MqttConnState { disconnected, connecting, connected, error }

class MqttProvider extends ChangeNotifier {
  final String host;
  final int port; 
  final String clientId;
  final String username;
  final String password;
  final String? wsUrl;
  final String cmdTopic; 
  final String telemetryBase; 

  MqttClient? _client;
  StreamSubscription<List<MqttReceivedMessage<MqttMessage>>>? _sub;

  MqttConnState state = MqttConnState.disconnected;
  String? lastError;

  final Map<String, dynamic> latestTelemetry = {};
  String? lastTopic;
  String? lastPayload;

  Timer? _reconnectTimer;
  bool _manualDisconnect = false;
  bool _connecting = false;

  MqttProvider({
    String? clientId,
    // 🟢 CRITICAL: Switch to EMQX Broker
    this.host = "broker.emqx.io", 
    this.port = 1883,
    this.username = "",
    this.password = "",
    this.wsUrl, 
    this.cmdTopic = "k9ops/trainer/cmd",
    this.telemetryBase = "k9ops/dog",
  }) : clientId = clientId ?? "k9ops_flutter_${DateTime.now().millisecondsSinceEpoch}";

  bool get isConnected => _client?.connectionStatus?.state == MqttConnectionState.connected;

  Future<void> connect() async {
    if (_connecting) return;
    _connecting = true;
    _manualDisconnect = false;
    state = MqttConnState.connecting;
    notifyListeners();

    try { await _sub?.cancel(); } catch (_) {}
    _sub = null;
    try { _client?.disconnect(); } catch (_) {}
    _client = null;

    // 🟢 CRITICAL: EMQX uses Port 8083 for WebSockets
    int targetPort = kIsWeb ? 8083 : port; 
    // We use 'ws' (unencrypted) because it is faster and more stable for testing
    String transportScheme = kIsWeb ? 'ws' : 'tcp';
    
    // Explicitly build the WebSocket URL
    final webWsUrl = '$transportScheme://$host:$targetPort/mqtt'; 

    debugPrint("🔌 Connecting to: $webWsUrl");

    final c = createPlatformClient(
      host: host,
      port: targetPort, 
      clientId: clientId,
      wsUrl: webWsUrl, 
    );

    c.logging(on: true); 

    final connMsg = MqttConnectMessage()
        .withClientIdentifier(clientId)
        .startClean()
        .withWillQos(MqttQos.atLeastOnce);

    if (username.isNotEmpty) {
      connMsg.authenticateAs(username, password);
    }

    c.connectionMessage = connMsg;
    _client = c;

    try {
      await c.connect();
    } catch (e) {
      debugPrint("❌ MQTT Connection Failed: $e");
      state = MqttConnState.error;
      lastError = e.toString();
      notifyListeners();
      _connecting = false;
      _scheduleReconnect();
      return;
    }

    if (c.connectionStatus?.state == MqttConnectionState.connected) {
      debugPrint("✅ MQTT Connected!");
      _sub = c.updates?.listen(_onMessage);
      subscribe('$telemetryBase/+/telemetry');
      state = MqttConnState.connected;
    } else {
      state = MqttConnState.error;
      _scheduleReconnect();
    }
    
    notifyListeners();
    _connecting = false;
  }

  Future<void> disconnect() async {
    _manualDisconnect = true;
    _reconnectTimer?.cancel();
    try { await _sub?.cancel(); } catch (_) {}
    try { _client?.disconnect(); } catch (_) {}
    state = MqttConnState.disconnected;
    notifyListeners();
  }

  void subscribe(String topic, {MqttQos qos = MqttQos.atLeastOnce}) {
    if (_client != null && isConnected) {
      _client!.subscribe(topic, qos);
    }
  }

  void sendCommand({
    required String target,
    required String cmd,
    dynamic value,
  }) {
    if (_client == null || !isConnected) {
      debugPrint("⚠️ Cannot send command: MQTT Disconnected");
      return;
    }

    final payloadMap = {
      "target": target,
      "command": cmd, 
      "value": value,
      "ts": DateTime.now().millisecondsSinceEpoch,
    };

    final payload = jsonEncode(payloadMap);
    final builder = MqttClientPayloadBuilder()..addString(payload);

    debugPrint("📤 Sending: $payload to $cmdTopic");
    _client!.publishMessage(cmdTopic, MqttQos.atLeastOnce, builder.payload!);
  }

  void _scheduleReconnect() {
    if (_manualDisconnect) return;
    _reconnectTimer?.cancel();
    _reconnectTimer = Timer.periodic(const Duration(seconds: 3), (_) async {
      if (!isConnected && !_connecting) await connect();
    });
  }

  void _onMessage(List<MqttReceivedMessage<MqttMessage>> events) {
    // Handle incoming telemetry
  }
}