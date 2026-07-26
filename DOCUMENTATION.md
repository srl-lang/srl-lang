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

*SRL (Serial Run Language) v0.1.0 - GNU General Public License v3.0*
