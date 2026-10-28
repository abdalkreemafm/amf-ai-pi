import 'dart:convert';
import 'dart:typed_data';
import 'package:crypto/crypto.dart';
import 'package:http/http.dart' as http;
import 'package:shared_preferences/shared_preferences.dart';

/// Cloud synchronization and entitlement cache.
///
/// This class deliberately does not implement cryptography in Dart. BLE
/// authentication must be provided by the platform/hardware secure channel;
/// fixed XOR keys in a mobile app are not security.
class HadeedSecurityVault {
  HadeedSecurityVault({Uri? endpoint})
      : _cloudEndpoint = endpoint ?? Uri.parse('https://api.hadeed-tech.com/v1/sync');

  final Uri _cloudEndpoint;
  String? _userToken;

  void setUserToken(String token) {
    if (token.trim().isEmpty) {
      throw ArgumentError('token must not be empty');
    }
    _userToken = token;
  }

  Uint8List generateHandshakeResponse(Uint8List challenge) {
    throw UnsupportedError(
      'Insecure fixed-key handshake removed; use BLE Secure Connections.',
    );
  }

  Uint8List decryptPayload(Uint8List encryptedPayload) {
    throw UnsupportedError(
      'XOR payload decryption removed; use an AEAD-secured BLE channel.',
    );
  }

  Future<bool> exchangeFingerprintsForPacks(
      List<Map<String, dynamic>> localLogs) async {
    final token = _userToken;
    if (token == null) return false;

    final anonymousData = localLogs.map((log) => <String, dynamic>{
          'peak': log['peak'],
          'slope': log['log_slope'],
          'harmony': log['harmony_ratio'],
          'phase': log['phase_angle'],
          'type': log['target_type'],
          'date': DateTime.now().toUtc().toIso8601String(),
        }).toList();

    try {
      final response = await http
          .post(
            _cloudEndpoint,
            headers: {
              'Content-Type': 'application/json',
              'Authorization': 'Bearer $token',
            },
            body: jsonEncode({'fingerprints': anonymousData}),
          )
          .timeout(const Duration(seconds: 10));

      if (response.statusCode >= 200 && response.statusCode < 300) {
        await _cacheEntitlement('BASALT_BYPASS_V2');
        return true;
      }
      return false;
    } on Exception {
      return false;
    }
  }

  Future<void> _cacheEntitlement(String packId) async {
    final prefs = await SharedPreferences.getInstance();
    final digest = sha256.convert(utf8.encode('$packId:HADEED_SALT_2026'));
    await prefs.setBool('entitlement_${digest.toString()}', true);
  }

  Future<bool> isPackUnlocked(String packId) async {
    final prefs = await SharedPreferences.getInstance();
    final digest = sha256.convert(utf8.encode('$packId:HADEED_SALT_2026'));
    return prefs.getBool('entitlement_${digest.toString()}') ?? false;
  }

  Future<void> syncAIWeightsFromCloud() async {
    throw UnsupportedError('Secure signed model update is not implemented yet.');
  }
}
