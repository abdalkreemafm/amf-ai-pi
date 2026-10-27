import 'package:flutter/material.dart';

void main() {
  runApp(const HadeedApp());
}

class HadeedApp extends StatelessWidget {
  const HadeedApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Hadeed',
      theme: ThemeData.dark(useMaterial3: true),
      home: const HadeedHomePage(),
    );
  }
}

class HadeedHomePage extends StatelessWidget {
  const HadeedHomePage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Hadeed')),
      body: const Center(
        child: Text('Hardware connection is not configured yet.'),
      ),
    );
  }
}
