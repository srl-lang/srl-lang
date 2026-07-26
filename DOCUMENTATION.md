# SRL (Serial Run Language) - Kapsamlı Dil ve Mimari Şartnamesi (Specification & Documentation)

**Sürüm:** `v0.1.0`  
**Mimari:** C++ Bytecode Virtual Machine (VM) & LLVM IR Ahead-of-Time (AOT) Standalone Compiler (`srlc`)  
**Lisans:** GNU General Public License v3.0 (GPLv3)

---

## 📑 İçindekiler
1. [Giriş ve Dil Felsefesi](#1-giriş-ve-dil-felsefesi)
2. [Temel Sözdizimi ve Veri Tipleri](#2-temel-sözdizimi-ve-veri-tipleri)
3. [Döngüler ve Akış Kontrolü](#3-döngüler-ve-akış-kontrolü)
4. [Fonksiyonlar ve Kapsam (Scope)](#4-fonksiyonlar-ve-kapsam-scope)
5. [Diziler ve Haritalar (Arrays & Maps)](#5-diziler-ve-haritalar-arrays--maps)
6. [C++ Tarzı Yapılar (Structs)](#6-c-tarzı-yapılar-structs)
7. [Lua Tarzı Metatable ve Metamethods](#7-lua-tarzı-metatable-ve-metamethods)
8. [Dijital Sinyal İşleme (DSP) & FFT Motoru](#8-dijital-sinyal-işleme-dsp--fft-motoru)
9. [Terminal Kullanıcı Arayüzü (TUI Modülü)](#9-terminal-kullanıcı-arayüzü-tui-modülü)
10. [Canlı Koda Müdahale (Live Hot-Reloading & State Retention)](#10-canlı-koda-müdahale-live-hot-reloading--state-retention)
11. [Müstakil LLVM Makine Kodu Derleyicisi (`srlc`)](#11-müstakil-llvm-makine-kodu-derleyicisi-srlc)
12. [CLI Araç Takımı Komut Rehberi](#12-cli-araç-takımı-komut-rehberi)

---

## 1. Giriş ve Dil Felsefesi

**SRL (Serial Run Language)**, C dilinin basitliği ve sistem performansını, Lua'nın esnek nesne ve prototip modeliyle harmanlayan, özellikle **gerçek zamanlı sinyal işleme (DSP)**, **ses sentezleme**, **canlı kod güncelleme (hot-reloading)** ve **bağımsız yerel makine kodu (native executable)** üretimi için tasarlanmış hibrit bir programlama dilidir.

### Temel Tasarım İlkeleri:
- **Kesintisiz Çalışma (Zero-Downtime Hot-Reloading):** Uygulama durdurulmadan `.srl` kaynak kodu değiştirildiğinde bellekteki değişken durumları (state) korunarak kod anında güncellenir.
- **Donanımsal Sinyal Performansı:** Dahili Cooley-Tukey Radix-2 FFT algoritması, filtreler ve osilötörler C++ hızında yerleşik olarak çalışır.
- **Çift Motor Desteği:** İster C++ Bytecode VM ile anında çalıştırın, ister `srlc` ile doğrudan LLVM IR ve x86_64 bağımsız `.exe` ikili dosyasına derleyin.

---

## 2. Temel Sözdizimi ve Veri Tipleri

SRL dilinde değişkenler `var` anahtar kelimesi ile tanımlanır. Dinamik tiplemeye (dynamic typing) sahiptir.

### Desteklenen Veri Tipleri:
- `NUMBER`: 64-bit kayan noktalı sayı (Double precision IEEE-754).
- `STRING`: UTF-8 metin dizileri.
- `BOOL`: `true` veya `false`.
- `NIL`: Boş/Tanımsız değer (`nil`).
- `ARRAY`: Dinamik boyutlu heterojen listeler.
- `MAP`: Anahtar-değer sözlükleri (Tables).
- `FUNCTION`: Birincil sınıf (First-class) fonksiyon nesneleri.

```srl
// Değişken Tanımlamaları
var ses_frekansi = 440;            // Number
var kanal_adi = "Sol Ses Kanalı";  // String
var aktif_mi = true;               // Bool
var veri_yok = nil;                // Nil

// Metin Birleştirme ve Tip Dönüşümü
var mesaj = kanal_adi + " -> Frekans: " + to_string(ses_frekansi) + " Hz";
print(mesaj);
```

---

## 3. Döngüler ve Akış Kontrolü

### Koşullu İfadeler (`if` / `else`):
```srl
var genlik = 0.85;

if genlik > 0.8 {
    print("Yüksek sinyal uyarısı!");
} else {
    print("Normal sinyal seviyesi.");
}
```

### `while` Döngüsü:
```srl
var i = 0;
while i < 5 {
    print("Adım: " + to_string(i));
    i = i + 1;
}
```

---

## 4. Fonksiyonlar ve Kapsam (Scope)

Fonksiyonlar `fn` anahtar kelimesi ile tanımlanır ve değer dönebilir (`return`).

```srl
fn genlik_hesapla(real, imag) {
    var mag = math_abs(real) + math_abs(imag);
    return mag;
}

var sonuc = genlik_hesapla(12.5, -4.2);
print("Genlik: " + to_string(sonuc));
```

---

## 5. Diziler ve Haritalar (Arrays & Maps)

SRL, yüksek performanslı dinamik diziler ve haritalar için dahili fonksiyonlar sunar:

### Dizi Fonksiyonları:
- `arr_new()`: Yeni boş dizi oluşturur.
- `arr_push(arr, val)`: Dizinin sonuna eleman ekler.
- `arr_get(arr, index)`: İndksteki elemanı okur.
- `arr_set(arr, index, val)`: İndekse değer yazar (sınır dışı indekslerde diziyi otomatik büyütür).
- `arr_len(arr)`: Dizi uzunluğunu döner.

```srl
var liste = arr_new();
arr_push(liste, 100);
arr_push(liste, 200);
arr_set(liste, 2, 300); // 2. indekse yazar

print("Dizi elemanı [1]: " + to_string(arr_get(liste, 1)));
print("Dizi boyutu: " + to_string(arr_len(liste)));
```

### Harita (Map/Sözlük) Fonksiyonları:
- `map_new()`: Yeni harita oluşturur.
- `map_set(map, key, val)`: Anahtar-değer çifti yazar.
- `map_get(map, key)`: Anahtarın değerini okur.
- `map_has(map, key)`: Anahtarın varlığını kontrol eder (`true`/`false`).

```srl
var oyuncu = map_new();
map_set(oyuncu, "isim", "Ahmet");
map_set(oyuncu, "skor", 1500);

print("Oyuncu: " + map_get(oyuncu, "isim") + " | Skor: " + to_string(map_get(oyuncu, "skor")));
```

---

## 6. C++ Tarzı Yapılar (Structs)

`struct` anahtar kelimesi ile özel yapılar tanımlayabilir ve bunlardan yapıcı fonksiyonlar (constructors) türetebilirsiniz:

```srl
// Struct Tanımı
struct Nokta2D { x, y }
struct Vektor3D { x, y, z }

// Yapılardan Nesne Türetme (Object Instantiation)
var p1 = Nokta2D(15, 25);
var v1 = Vektor3D(1.0, 2.5, 9.8);

print("P1 X: " + to_string(map_get(p1, "x")) + ", Y: " + to_string(map_get(p1, "y")));
```

---

## 7. Lua Tarzı Metatable ve Metamethods

SRL nesnelerine (Map) prototip tabanlı kalıtım ve özel operatör davranışları kazandırmak için Metatable desteği bulunur:

### Metatable Fonksiyonları:
- `setmetatable(obj, metatable)`: Nesneye metatable bağlar.
- `getmetatable(obj)`: Nesnenin metatable'ını döner.
- `rawget(obj, key)`: Metatable `__index` mekanizmasını atlayarak doğrudan nesneden okuma yapar.
- `rawset(obj, key, val)`: Doğrudan yazar.

### Desteklenen Metamethodlar:
- `__index`: Aranan alan nesnede bulunamadığında prototip haritasına düşer.
- `__add`, `__sub`, `__mul`, `__div`: Nesneler üzerinde `+`, `-`, `*`, `/` operatörleri kullanıldığında tetiklenir.

```srl
// Prototip Haritası (Base Class)
var Prototip = map_new();
map_set(Prototip, "tur", "SES_NESNESI");
map_set(Prototip, "ornekleme_hizi", 44100);

var meta = map_new();
map_set(meta, "__index", Prototip);

// Yeni Nesne
var ses_objesi = map_new();
map_set(ses_objesi, "kanal_adi", "Master");
setmetatable(ses_objesi, meta);

// "tur" alanı ses_objesi'nde yok, fakat metatable __index sayesinde Prototip'ten okunur:
print("Nesne Türü: " + map_get(ses_objesi, "tur")); // Çıktı: SES_NESNESI
```

---

## 8. Dijital Sinyal İşleme (DSP) & FFT Motoru

SRL dilinin en güçlü yanlarından biri dahili C++ hızındaki sinyal işleme kütüphanesidir:

| Fonksiyon | Açıklama |
| :--- | :--- |
| `dsp_sine(freq, sample_rate, num_samples)` | Belirtilen frekanta sinüs dalgası dizisi üretir. |
| `dsp_square(freq, sample_rate, num_samples)` | Kare dalga sinyali üretir. |
| `dsp_noise(num_samples)` | Beyaz gürültü (white noise) dizisi üretir. |
| `dsp_hann(num_samples)` | Hann pencere katsayıları dizisi döner. |
| `dsp_hamming(num_samples)` | Hamming pencere katsayıları dizisi döner. |
| `dsp_lowpass(signal, cutoff, sample_rate)` | Alçak geçiren filtre uygular. |
| `dsp_fft(real_array, [imag_array])` | Cooley-Tukey Radix-2 FFT hesabı yapar. `{"real": [...], "imag": [...]}` döner. |
| `dsp_ifft(real_array, imag_array)` | Ters FFT (IFFT) ile sinyali geri kurar. |
| `dsp_magnitude(real, imag)` | Spektrum genliğini hesaplar ($|Z| = \sqrt{R^2 + I^2}$). |
| `dsp_plot(signal, height, title)` | Terminalde interaktif ASCII dalga/spektrum grafiği çizer. |

### Örnek DSP & FFT Kullanımı:
```srl
// 1. 440 Hz Sinüs Dalgası Üret
var signal = dsp_sine(440, 8000, 64);
dsp_plot(signal, 8, "440Hz Sinus Dalgasi");

// 2. FFT Hesabı Yap ve Spektrumu Çiz
var fft_res = dsp_fft(signal);
var mag = dsp_magnitude(map_get(fft_res, "real"), map_get(fft_res, "imag"));
dsp_plot(mag, 8, "FFT Frekans Spektrumu");
```

---

## 9. Terminal Kullanıcı Arayüzü (TUI Modülü)

Terminalde canlı paneller, çerçeveler ve renkli arayüzler çizmek için dahili TUI komutları:

- `tui_init()` / `tui_reset()`: Terminal modunu başlatır / sıfırlar.
- `tui_clear()`: Ekranı temizler.
- `tui_color("cyan")`: ANSI rengini ayarlar (`red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`).
- `tui_move(row, col)`: İmleci belirli satır/sütuna taşır.
- `tui_box(row, col, width, height, title)`: Renkli çerçeve çizer.
- `tui_progress(row, col, width, percent)`: İlerleme çubuğu (progress bar) çizer.
- `tui_get_key()`: Klavyeden basılan tuşu anlık okunur.

---

## 10. Canlı Koda Müdahale (Live Hot-Reloading & State Retention)

SRL motoru, uygulamanızı kapatmadan kod güncellemeyi destekler.

```bash
srl watch dosyam.srl
```

Bu modda:
1. `dosyam.srl` dosyasını bir kod düzenleyicide değiştirip kaydettiğiniz an SRL motoru değişikliği algılar.
2. Bellekteki küresel değişkenlerinizi ve uygulamanın mevcut durumunu (**state**) koruyarak yeni koda anında geçer.

---

## 11. Müstakil LLVM Makine Kodu Derleyicisi (`srlc`)

SRL dilindeki kodları sanal makineden bağımsız, doğrudan x86_64 makine koduna ve tek bir bağımsız `.exe` ikili dosyasına derlemek için `srlc` derleyicisi kullanılır.

### Derleme Mantığı:
1. SRL AST düğümleri okunur.
2. `srlc` LLVM IR (`.ll`) SSA talimat kodlarını üretir.
3. LLVM makine kodu motoru doğrudan yüksek performanslı bağımsız `.exe` oluşturur.

```bash
# Doğrudan Makine Koduna Derleme:
srl build betik.srl -o uretilen_uygulama.exe

# İsteğe Bağlı LLVM IR Çıktısını İnceleme:
srlc betik.srl --emit-llvm
```

---

## 12. CLI Araç Takımı Komut Rehberi

Sisteminizde `srl` komutunu kullanarak tüm işlemleri tek bir noktadan yönetebilirsiniz:

| Komut | İşlev |
| :--- | :--- |
| `srl run <dosya.srl>` | Betiği SRL Sanal Makinesinde (Bytecode VM) çalıştırır. |
| `srl build <dosya.srl> [-o binary.exe]` | Betiği müstakil yerel x86_64 `.exe` makine koduna derler. |
| `srl watch <dosya.srl>` | Canlı koda müdahale (Hot-Reloading) modunda çalıştırır. |
| `srl version` | SRL dil araç takımı ve derleyici sürümünü gösterir. |
| `srl help` | Komut yardım menüsünü ekrana basar. |

---

## 13. Genişletilmiş Standart Kütüphaneler (Standard Libraries & Audio Engine)

SRL v0.2.0 ile gelişmiş ses motoru (Audio Engine), masaüstü diyalogları (GUI Dialogs), dosya ve dizin tarama (`dir_*`), matematik, metin işleme ve modüler `import` yetenekleri eklenmiştir:

### A. Yerleşik Ses Motoru (`audio_*`)
Windows MCI altyapısı sayesinde ek bağımlılık olmadan MP3, WAV ve MID dosyalarını oynatır:

| Fonksiyon | Açıklama |
| :--- | :--- |
| `audio_play(filepath)` | Belirtilen MP3/WAV dosyasını oynatır. |
| `audio_pause()` | Oynatmayı duraklatır. |
| `audio_resume()` | Duraklatılan parçayı devam ettirir. |
| `audio_stop()` | Parçayı durdurur ve dosyayı kapatır. |
| `audio_set_volume(0..100)` | Ses yüksekliğini ayarlar. |
| `audio_get_position()` | Oynatılan parçanın anlık saniyesini döner. |
| `audio_get_length()` | Parçanın toplam uzunluğunu (saniye) döner. |
| `audio_seek(seconds)` | Parçada belirli saniyeye atlar. |
| `audio_is_playing()` | Parçanın çalma durumunu döner (`true`/`false`). |
| `audio_beep(freq, duration_ms)` | Donanımsal sinyal/bip sesi üretir. |

### B. Masaüstü Arayüz Diyalogları (`gui_*`)
| Fonksiyon | Açıklama |
| :--- | :--- |
| `gui_file_dialog_open(title, filter)` | Masaüstü yerel dosya seçim penceresini açar (Seçilen dosya yolunu döner). |
| `gui_file_dialog_save(title, filter)` | Kaydetme dosyası diyalog penceresini açar. |
| `gui_msgbox(title, message, type)` | Yerel Win32 diyalog kutusu çıkarır (`info`, `warning`, `error`, `question`). |

### C. Dizin & Dosya Sistemi (`dir_*` & `file_*`)
| Fonksiyon | Açıklama |
| :--- | :--- |
| `dir_list(path)` | Dizin içindeki tüm dosya/klasör isimlerini SRL dizisi (`Array`) olarak döner. |
| `dir_list_ext(path, extension)` | Sadece belirli uzantıdaki (örn: `.mp3`) dosyaları listeler. |
| `dir_exists(path)` / `dir_create(path)` | Dizin varlığını kontrol eder / Yeni dizin oluşturur. |
| `file_append(path, content)` | Var olan dosyanın sonuna metin ekler. |
| `file_remove(path)` / `file_size(path)` | Dosyayı siler / Dosya boyutunu (bayt) döner. |
| `file_copy(src, dst)` | Dosyayı kopyalar. |

### D. Matematik & String Kütüphanesi (`math_*` & `str_*`)
- **Matematik:** `math_sin`, `math_cos`, `math_tan`, `math_sqrt`, `math_pow`, `math_exp`, `math_log`, `math_floor`, `math_ceil`, `math_round`, `math_min`, `math_max`, `math_clamp`, `math_random()`, `math_random_range(min, max)`, `math_pi()`.
- **Metin İşleme:** `str_upper`, `str_lower`, `str_trim`, `str_find`, `str_replace`, `str_split(str, delim)`, `str_contains`, `str_starts_with`, `str_ends_with`.

### E. Modüler Kod Yükleme (`import`)
```srl
import("std/audio.srl");
import("std/ui.srl");

var player = audio_player_new();
```

---

## 14. C++ Sistem Düzeyi Yetenekler (FFI, GFX, NET, SYS, Threads)

SRL diline C++ dilinin sunduğu tam donanımlı sistem seviyesi yetenekler kazandırılmıştır:

### A. Dış Kütüphane Çağırma (C/C++ FFI - `ffi_*`)
SRL içerisinden herhangi bir dış Win32 / C/C++ `.dll` dosyası dinamik olarak yüklenebilir ve fonksiyonları çağrılabilir:
- `ffi_load(dll_path)` -> DLL kütüphanesini belleğe yükler (Handle ID döner).
- `ffi_free(handle)` -> Yüklenen kütüphaneyi serbest bırakır.
- `ffi_call(handle, symbol_name, return_type, args)` -> Kütüphanedeki C fonksiyonunu çalıştırır.

### B. Yerel 2D Grafik Motoru (`gfx_*`)
Win32 GDI ve çift arabellekleme (double buffering) ile SRL dilinde masaüstü grafik penceresi ve 2D oyunlar/çizimler yapılabilir:
- `gfx_window_create(title, width, height)` -> Yerel 2D çizim penceresi oluşturur.
- `gfx_clear(color_name)` -> Pencereyi temizler (`black`, `darkgray`, `white`, `red`, `green`, `blue`, vb.).
- `gfx_draw_line(x1, y1, x2, y2, color)`, `gfx_draw_rect(...)`, `gfx_fill_rect(...)`.
- `gfx_draw_circle(cx, cy, r, color)`, `gfx_fill_circle(...)`, `gfx_draw_text(x, y, text, color)`.
- `gfx_present()` -> Çift arabelleği (double buffer) ekrana yansıtır (60 FPS animasyonlar için).
- `gfx_is_open()`, `gfx_poll_events()`, `gfx_close()`.

### C. Ağ & Soket Motoru (`net_*`)
Winsock2 altyapısı ile HTTP istekleri ve TCP iletişimi:
- `net_http_get(url_host, path)` -> HTTP GET isteği gönderir ve yanıt metnini döner.
- `net_tcp_connect(host, port)` -> TCP istemci soketi açar.
- `net_tcp_send(sock_id, data)` / `net_tcp_recv(sock_id, max_bytes)` -> Veri gönderir / alır.
- `net_tcp_close(sock_id)` -> Soketi kapatır.

### D. Sistem & Süreç Yönetimi (`sys_*`)
- `sys_exec(command)` -> Sistem kabuk komutunu çalıştırır ve stdout çıktısını string olarak döner.
- `sys_cpu_count()` -> İşlemcinin çekirdek sayısını döner.
- `sys_memory_usage()` -> Uygulamanın RAM bellek kullanımını (bayt) döner.
- `sys_pid()` -> Süreç ID'sini döner.
- `sys_env_get(name)` / `sys_env_set(name, value)` -> Ortam değişkenlerini okur/yazar.

### E. Çoklu İş Parçacığı (`thread_*`)
- `thread_create(fn_name)` -> SRL fonksiyonunu arka planda asenkron `std::thread` olarak çalıştırır.
- `thread_join(thread_id)` -> Arka plan iş parçacığının bitmesini bekler.

---

## 15. Temel Kütüphaneler (JSON, Veritabanı, Kriptografi & GUI Widgets)

SRL v0.3.0 ile yazılımlar için elzem olan JSON, Veritabanı, Kriptografi ve Masaüstü GUI Widget kütüphaneleri eklenmiştir:

### A. JSON Motoru (`json_*`)
- `json_parse(json_str)` -> JSON formatındaki metinleri SRL Map/Array yapısına dönüştürür.
- `json_stringify(value)` -> SRL değişken ve harita yapılarını biçimlendirilmiş JSON metnine dönüştürür.

### B. Kalıcı Yerel Veritabanı (`db_*`)
- `db_open(filepath)` -> Dosya tabanlı anahtar-değer veritabanını açar/oluşturur.
- `db_set(db_handle, key, value)` -> Anahtar-değer saklar ve dosyaya yazar.
- `db_get(db_handle, key)` -> Anahtarın değerini okur.
- `db_has(db_handle, key)` / `db_delete(db_handle, key)` -> Anahtar varlığı kontrolü / silme.
- `db_close(db_handle)` -> Veritabanını kaydeder ve kapatır.

### C. Kriptografi & Güvenlik (`crypto_*`)
- `crypto_sha256(str)` -> SHA-256 özet hash değerini döner.
- `crypto_md5(str)` -> MD5 özet hash değerini döner.
- `crypto_base64_encode(str)` / `crypto_base64_decode(str)` -> Base64 kodlar ve çözer.

### D. Masaüstü GUI Widget Araç Takımı (`std/widget.srl`)
- `widget_button_create(x, y, w, h, text, color)` / `widget_button_render(btn)` -> Tıklanabilir buton bileşenleri.
- `widget_slider_create(x, y, w, min, max, val)` / `widget_slider_render(slider)` -> Ses ve değer ayar slider'ları.

---

*SRL (Serial Run Language) v0.3.0 - GNU General Public License v3.0*



