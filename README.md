# Classic Tetris for Android

Kotlin/Android の描画・入力層と、C++17 のゲームエンジンを JNI で接続した Tetris アプリです。Android 24 以降に対応します。

## Build

JDK 17 と Android SDK/NDK を用意して、リポジトリルートから実行します。

```bash
cd tetris
./gradlew assembleDebug
```

Gradle が Kotlin、Android resources、CMake/NDK の `libtetris.so`、APK の順にまとめてビルドします。ローカル Debug APK は `tetris/app/build/outputs/apk/debug/app-debug.apk` に生成されます。

Unix 版 `gradlew` は初回実行時にピン留めされた Gradle Wrapper JAR を取得し、SHA-256 を検証します。これはPull Requestにバイナリを含めずにWrapperを利用するためで、初回のみ `curl` とネットワーク接続が必要です。

## Tests and lint

Android unit tests:

```bash
cd tetris
./gradlew testDebugUnitTest
```

Android lint:

```bash
cd tetris
./gradlew lintDebug
```

Android Framework に依存しない native game-engine tests:

```bash
cmake -S tetris/native-tests -B tetris/native-tests/build
cmake --build tetris/native-tests/build
ctest --test-dir tetris/native-tests/build --output-on-failure
```

## GitHub Actions

Pull Request と `main` branch への push では、Native C++ Tests、Android Unit Tests、Lint、Debug APK Build を順番に実行します。すべて成功した場合だけ APK を artifact として保存します。

Artifact 名は `ClassicTetris-YYYYMMDD-debug`、含まれるファイル名は `ClassicTetris-YYYYMMDD-debug.apk` です。日付は workflow 実行時の日本時間（Asia/Tokyo）です。Release signing、AAB、Google Play への公開は対象外です。
