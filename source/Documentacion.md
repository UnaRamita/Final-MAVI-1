# Documentación técnica — Tp Final MAVI (Plataformas 2D)

Este documento describe la arquitectura del proyecto, cómo se aplican los conceptos de la
cursada (POO, física, colisiones, sonido, HUD, menús) y el detalle de cada clase programada
hasta el momento. Sirve como base para el "documento explicativo" que pide la consigna.

## 1. Resumen del proyecto

Es un plataformas 2D hecho con C++ y Raylib. El jugador se mueve por un nivel de ejemplo
esquivando/eliminando enemigos con distintos tipos de movimiento, recolecta ítems y armas,
y debe resolver una puerta bloqueada con un botón para llegar al final del nivel.

**Género:** Platformer simple (con elementos de Shooter 2D por las armas a distancia).

**Controles:**

| Acción | Tecla |
|---|---|
| Moverse | Flechas o `A` / `D` |
| Saltar | `Espacio`, `↑` o `W` |
| Atacar (con arma equipada) | `F` |
| Reiniciar (en pantalla de fin) | `R` |
| Volver al menú | `ESC` |

## 2. Arquitectura general

```
main.cpp
  └── game            (ventana, audio, loop principal)
        └── scene      (maquina de estados: Menu / Opciones / Juego)
              └── level        (el "juego principal": nivel de ejemplo)
                    ├── player          (jugador: fisica, vida, arma equipada)
                    │     └── weapon    (arma actualmente equipada)
                    ├── enemyManager
                    │     └── enemy (base)
                    │           ├── enemyLinear       (MRU)
                    │           ├── enemyAccelerated  (MRUA)
                    │           └── enemyCircular     (MCU)
                    ├── item[]          (coleccionables)
                    ├── weaponPickup[]  (armas tiradas en el nivel)
                    ├── proyectile[]    (disparos del bastón)
                    ├── door[]          (puertas bloqueadas)
                    ├── button[]        (botones que desbloquean puertas)
                    ├── hud             (texto en pantalla)
                    └── soundManager    (efectos de sonido del gameplay)

Collisions (namespace libre, sin estado) — usado por player, item, weaponPickup,
button, enemyManager y proyectile para resolver colisiones AABB y circulo-rectangulo.
```

Cada clase vive en su propio par de archivos `.h`/`.cpp`, siguiendo la convención ya
existente en el proyecto (nombres de clase en minúscula: `game`, `scene`, `player`, etc.).

## 3. Diagrama de estados

**Nivel de `scene` (menú):**

```
MenuInicio ──(Play)──> Jugando ──(ESC)──> MenuInicio
     │
     └──(Options)──> Opciones ──(Back)──> MenuInicio
```

**Nivel de `level` (dentro de "Jugando"):**

```
PLAYING ──(vidas = 0)──> GAMEOVER ──(R)──> PLAYING (reinicio)
PLAYING ──(items = 0)──> VICTORY  ──(R)──> PLAYING (reinicio)
(GAMEOVER | VICTORY | PLAYING) ──(ESC)──> vuelve a scene (MenuInicio)
```

`scene` solo sabe si está en el estado "Juego" (`sceneN == 2`); toda la lógica de
Game Over / Victoria / reinicio vive adentro de `level`, que se dibuja y actualiza
sobre esa misma pantalla.

## 4. Aplicación de los conceptos de la cursada

### POO (clases en .h/.cpp)
Todas las clases separan declaración (`.h`) de implementación (`.cpp`). Hay herencia real
en el árbol de enemigos: `enemy` es una clase base abstracta (`Update` es virtual puro) y
`enemyLinear`, `enemyAccelerated` y `enemyCircular` la heredan, cada una con su propia
implementación de movimiento (polimorfismo). `enemyManager` guarda punteros a la base
(`std::vector<enemy*>`) y llama `Update()`/`Draw()` sin saber de qué subtipo se trata.

### MRU (movimiento rectilíneo uniforme)
Velocidad constante, `posición += velocidad * tiempo`.
- [Player.cpp](Player.cpp) `MoveX()`: el jugador se mueve a velocidad constante en X mientras se mantiene la tecla.
- [Enemy.cpp](Enemy.cpp) `enemyLinear::Update()`: patrulla entre dos puntos a velocidad constante, invirtiendo el signo al llegar a los límites.
- [Proyectile.cpp](Proyectile.cpp) `Update()`: el disparo del bastón viaja en línea recta a velocidad constante hasta salir de pantalla o impactar.

### MRUA (movimiento rectilíneo uniformemente acelerado)
Velocidad que cambia con una aceleración constante, `velocidad += aceleración * tiempo`.
- [Player.cpp](Player.cpp) `Update()`: la velocidad vertical (`vel.y`) se ve afectada por la gravedad todos los frames (`vel.y += gravity * dt`); el salto le da un impulso inicial negativo y la gravedad la va frenando/invirtiendo.
- [Enemy.cpp](Enemy.cpp) `enemyAccelerated::Update()`: acelera horizontalmente hacia el jugador (`vel.x += dir * accel * dt`) hasta un tope de velocidad máxima.

### MCU (movimiento circular uniforme)
Ángulo que crece a velocidad angular constante, posición sobre una circunferencia.
- [Enemy.cpp](Enemy.cpp) `enemyCircular::Update()`: `angle += angularSpeed * dt`, y la posición se calcula con `centro + radio * (cos(angulo), sin(angulo))`.

### Colisiones
Implementadas a mano (no son simples llamadas a funciones de Raylib) en
[Collisions.h](Collisions.h) / [Collision.cpp](Collision.cpp):
- `CheckAABB(a, b)`: colisión rectángulo-rectángulo por superposición de ejes. Se usa entre
  jugador/plataformas/puertas, jugador/enemigos, ataque cuerpo a cuerpo/enemigos, proyectil/enemigos, jugador/armas y jugador/botones.
- `CheckCircle(...)`: círculo-círculo, por distancia entre centros vs. suma de radios (queda disponible para reutilizar aunque el nivel actual no la necesita).
- `CheckCircleRect(...)`: círculo-rectángulo (el punto del rectángulo más cercano al centro del círculo), usada para que los ítems circulares detecten al jugador (que es un rectángulo).

La colisión del jugador contra el mundo se resuelve **por eje** (primero X, después Y) en
`player::MoveX()` / `player::MoveY()`: es el método estándar en plataformeros y es lo que
permite que una puerta cerrada bloquee de verdad el paso por los costados (no solo que el
jugador "aterrice" encima).

### Sonido
- Música: la maneja `scene` directamente (`menuMusic` / `gameMusic`, sin cambios respecto
  al proyecto original) con `LoadMusicStream` / `PlayMusicStream` / `UpdateMusicStream`.
- Efectos de sonido: los administra [SoundManager.h](SoundManager.h) / [SoundManager.cpp](SoundManager.cpp).
  Como todavía no hay archivos de audio propios para los efectos, se generan por código
  (`GenerateTone`, un barrido de frecuencia con envolvente) usando `LoadSoundFromWave` y
  `PlaySound`/`SetSoundVolume` de Raylib. Cada sonido tiene, comentada arriba, la línea
  `LoadSound("Assets/Sounds/...")` lista para descomentar el día que se agregue el archivo
  real (ver sección 7).

### HUD
[Hud.h](Hud.h) / [Hud.cpp](Hud.cpp) dibuja con `DrawText()`/`TextFormat()`: vidas, puntaje,
ítems recolectados/total, arma equipada y el tiempo transcurrido (arriba a la derecha).

### Menús / estados
- `scene` ya traía el menú de inicio (Play/Options/Exit) armado; no se tocó su lógica.
- Se agregó la conexión con `level` para el estado "Juego", y el propio `level` maneja
  Game Over / Victoria con reinicio (`R`) y vuelta al menú (`ESC`), cumpliendo el diagrama
  de estados pedido por la consigna (Menú → Jugando → GameOver/Victoria → Reinicio).

### Optimización
`enemyManager::Draw()` sólo dibuja los enemigos cuya hitbox interseca la pantalla
(`CheckCollisionRecs` contra el rectángulo de pantalla), en vez de dibujar todos siempre.
En este nivel de ejemplo (una sola pantalla) el impacto es mínimo, pero deja el patrón
listo para cuando el nivel crezca más allá de una pantalla (con cámara).

## 5. Detalle de clases

### `game` — [Game.h](Game.h) / [Game.cpp](Game.cpp)
Controlador de más alto nivel: crea la ventana, inicializa el dispositivo de audio y corre
el loop (`Run()`), delegando todo el contenido a `scene`.

### `scene` — [Scene.h](Scene.h) / [Scene.cpp](Scene.cpp)
Máquina de estados del menú (sin cambios en su lógica interna): `sceneN` indica si se está
en Menú (1), Juego (2) u Opciones (4). Dueña de la música y de una instancia de `level`
(`gameLevel`), que inicializa la primera vez que se aprieta "Play" y reinicia (`Reset()`)
las veces siguientes. También reenvía el volumen de efectos configurado en Opciones al
`SoundManager` del nivel.

### `level` — [Level.h](Level.h) / [Level.cpp](Level.cpp)
El "juego principal": arma el nivel de ejemplo (suelo, plataformas, enemigos, ítems, armas,
puerta y botón), actualiza todo cada frame, resuelve las interacciones entre objetos
(daño, recolección, ataques) y dibuja el mundo + HUD + pantallas de fin.

### `player` — [Player.h](Player.h) / [Player.cpp](Player.cpp)
- Física: MRU en X, MRUA (gravedad) en Y, salto con impulso fijo.
- Colisión contra sólidos (plataformas + puertas cerradas) resuelta por eje.
- Vidas (3 al empezar), invulnerabilidad temporal tras recibir daño (parpadea).
- Animación simple ciclando 4 sprites mientras se mueve.
- Tiene un `weapon` propio: equipa/ataca a través de él.

### `enemy` (+ `enemyLinear`, `enemyAccelerated`, `enemyCircular`) — [Enemy.h](Enemy.h) / [Enemy.cpp](Enemy.cpp)
Clase base abstracta + 3 subclases, una por cada tipo de movimiento pedido por la
consigna (ver sección 4). Todas comparten posición, tamaño, textura y estado vivo/muerto.

### `enemyManager` — [EnemyManager.h](EnemyManager.h) / [EnemyManager.cpp](EnemyManager.cpp)
Crea y administra los 3 enemigos del nivel de ejemplo (slime lineal, perro acelerado,
abeja circular). Expone `CheckPlayerCollision()` (daño de contacto) y `AttackRect()`
(usada por la espada y por los proyectiles para matar enemigos en un área).

### `item` — [Item.h](Item.h) / [Item.cpp](Item.cpp)
Coleccionable circular (moneda animada de `Art`; su colisión sigue siendo un círculo).
Suma puntaje al tocarlo; cuando todos están recolectados se gana el nivel.

### `weapon` / `weaponPickup` — [Weapons.h](Weapons.h) / [Weapons.cpp](Weapons.cpp)
- `weapon`: el arma que porta el jugador. Solo puede haber **una equipada a la vez**:
  `Equip()` reemplaza directamente el tipo anterior. Maneja el cooldown de ataque y el
  dibujo del arma junto al jugador. Tipos actuales: `SWORD` (cuerpo a cuerpo) y `STAFF`
  (a distancia, dispara `proyectile`).
- `weaponPickup`: el arma tirada en el nivel. Al tocarla, `level` llama
  `player::EquipWeapon()` con su tipo.

### `proyectile` — [Proyectile.h](Proyectile.h) / [Proyectile.cpp](Proyectile.cpp)
Disparo del bastón mágico: se mueve en línea recta (MRU) hasta salir de pantalla o
impactar contra un enemigo (usa `enemyManager::AttackRect()`).

### `door` / `button` — [Door.h](Door.h)/[Door.cpp](Door.cpp), [Button.h](Button.h)/[Button.cpp](Button.cpp)
- `button`: rectángulo en el suelo/plataforma; al tocarlo con la hitbox del jugador se
  marca como presionado (una sola vez) y desbloquea la puerta asociada por índice.
- `door`: mientras está bloqueada, se agrega a la lista de sólidos contra los que colisiona
  el jugador (bloquea el paso); al desbloquearse deja de ser sólida y se dibuja distinta
  (solo el marco).

### `hud` — [Hud.h](Hud.h) / [Hud.cpp](Hud.cpp)
Dibuja vidas, puntaje, ítems recolectados/total, arma equipada y tiempo transcurrido.

### `soundManager` — [SoundManager.h](SoundManager.h) / [SoundManager.cpp](SoundManager.cpp)
Genera y reproduce los efectos de sonido del gameplay (salto, moneda, golpe, disparo,
botón, equipar arma, victoria, game over). Ver sección 7 para reemplazarlos por archivos
reales.

### `Collisions` — [Collisions.h](Collisions.h) / [Collision.cpp](Collision.cpp)
Funciones libres (sin estado) de colisión: AABB, círculo-círculo y círculo-rectángulo.

## 6. El nivel de ejemplo

Pensado sobre una resolución de 1920×1080 (la ventana no es redimensionable):

- **Suelo**: franja completa en `y=1000` a `1080`.
- **4 plataformas flotantes** escalonadas para practicar el salto (los saltos
  fueron calculados para que la altura máxima de salto, ~205px, alcance holgadamente
  cada salto entre suelo → P1 → P2 → P3 → P4).
- **Enemigos**: un slime (MRU, patrulla en el suelo), un perro (MRUA, persigue al jugador
  cuando está cerca) y una abeja (MCU, orbita un punto fijo en el aire).
- **6 ítems** repartidos por el suelo y las plataformas.
- **Espada** cerca del spawn, fácil de conseguir para pelear contra el slime.
- **Puerta** que bloquea el tramo final del nivel (una columna sólida completa, pensada
  para que no se pueda saltar por arriba).
- **Botón** arriba de la segunda plataforma (P2) que desbloquea esa puerta.
- **Bastón mágico** y el último ítem, ubicados detrás de la puerta como recompensa.

## 7. Assets

**Sprites:** todo el arte (personaje, enemigos, jefe, armas, ítems, puertas, botones, fondos y
menús) ya sale de archivos en `Assets/Sprites/`. El arte del castillo se carga con el módulo
`Art` (ver la sección "Arte del castillo y animaciones"); el personaje usa
`Assets/Sprites/Character/PlayerSheet.png` (11 cuadros de 64x128).

**Música y golpes de espada:** `Assets/Music/` (menú y juego) y `Assets/Fx/` (tres tajos de espada).

**Sonidos generados por código** (en [SoundManager.cpp](SoundManager.cpp), función `Init()`):
salto, moneda, golpe, victoria, game over, disparo, botón y equipar arma se sintetizan con
`GenerateTone(...)`. Cada uno tiene arriba una línea `//xSound = LoadSound("Assets/Sounds/x.wav");`
comentada: para usar audio propio, comentá la línea de `GenerateTone(...)` y descomentá/completá
la de `LoadSound(...)` (el `UnloadSound` ya está en `Unload()`).

## 8. Cómo compilar y correr

Abrir `RaylibTemplate.sln` con Visual Studio (o compilar con MSBuild) en configuración
`Debug`/`Release`, plataforma `x64` o `Win32`. El proyecto espera `include/raylib` y
`lib/raylib/<arch>` un nivel arriba de `source/` (ya están en el repo). El ejecutable
busca los assets con rutas relativas (`Assets/...`), así que hay que correrlo con la
carpeta `source` como directorio de trabajo (Visual Studio ya lo configura así al debuggear).

## 9. Posibles próximos pasos

- Reemplazar los sonidos generados por código por archivos de audio propios (ver sección 7).
- Agregar más plataformas/niveles, o una cámara si el nivel crece más allá de una pantalla.
- Dar más vida (`SetHealth()`) a otros enemigos además del slime, si se quiere más dificultad.

---

## Arte del castillo y animaciones (módulo `Art`)

Todo el arte nuevo se carga **una sola vez** en `Art::Load()` (`Art.h` / `Art.cpp`, llamado desde `Level::Init` y `enemyManager::Init`) y se libera en `Art::Unload()` (`Level::Unload`). Las clases (`door`, `button`, `hazard`, `stone`, `item`, `movingPlatform`, `enemy`, `boss`) solo dibujan con esas texturas compartidas.

**Ambientaciones** (`Art::SetTheme(nivel)`, se llama en `Level::BuildLevel`): salón (niveles 0-3), mazmorra (4-6), torre (7-10) y trono (11). Cada una tiene `Assets/Sprites/Map/<Nombre>/` con `fondo.png` (1920x1080), `piso.png` (tile 128x80), `plat_izq/medio/der.png`. `Art::DrawSolid` dibuja cualquier rectángulo de colisión: alto >= 60 = piso, el resto = plataforma (tapas + medio repetido). `Art::DrawBackground` agrega los halos, estandartes y antorchas animadas.

**Objetos** (`Assets/Sprites/Others/Castle/`): puerta de rastrillo (8 cuadros, la animación arranca al `Unlock`), botón (2 cuadros), pinchos (4 cuadros; las trampas temporizadas avisan antes de salir), plataforma móvil, orbes del objetivo, moneda.

**Enemigos** (`Assets/Sprites/Enemy/Anim/`): hojas de cuadros en fila, dibujadas con `Art::DrawFrame` (ancla abajo-centro de la hitbox, o centro para la abeja). Las hitboxes no cambiaron.
- `enemy::AdvanceAnim` avanza el reloj de animación y de muerte; `Kill()` deja de contar/colisionar pero el enemigo se sigue dibujando durante su animación de muerte (slime: salpicadura; el resto: humo).
- Perro y orco miran a la izquierda en el sprite original; se espejan según hacia dónde van.
- `enemyRanged` (orco): al tener al jugador en rango se frena y reproduce `orc_cast` (6 cuadros); el proyectil sale en el cuadro 5 desde la mano (`ORC_SHOT_FRAME`).
- `boss`: `boss1_*` en fase de ataque, `boss2_*` (versión gris) en la fase vulnerable; se agacha mientras carga un ataque y salta/golpea al soltarlo.

**Ventana:** `Game::Inits` abre la ventana sin bordes, en (0,0) y de 1920x1080, para que no la tape la barra de tareas ni la barra de título.

## Ajustes de jugabilidad (tanda posterior)

- **Salto:** `jumpForce = -1040` (~237 px de altura) para llegar a las plataformas que estan 200 px mas arriba.
  Es un salto variable (`player::CutJumpIfReleased`): el salto sale completo al apretar y, si se suelta la tecla mientras
  todavia sube, la velocidad de subida se limita a -500 px/s. Un toque rapido da ~70 px, soltar a mitad de camino da una
  altura intermedia y mantenerla da los ~237 px. No hay "carga" previa.
- **Armas con E:** hay que pararse sobre el arma y apretar E (`weaponPickup::CheckPickup`); aparece el cartel "E - Recoger".
- **Menus:** `Art::DrawMenuBackground()` desliza el salon del castillo de izquierda a derecha (45 px/s, panorama con copia espejada).
- **Plataformas de un solo sentido:** `player::Update` recibe dos listas: `solids` (puertas cerradas, macizas) y `oneWay`
  (plataformas fijas y moviles). Las `oneWay` solo frenan al jugador si cae sobre ellas y sus pies venian por encima del borde
  superior; desde abajo o de costado se atraviesan.
- **Animacion del personaje:** `Assets/Sprites/Character/PlayerSheet.png`, 11 cuadros de 64x128 (0-1 reposo, 2-7 caminata,
  8 salto subiendo, 9 pico, 10 cayendo). El cuadro se elige en `player::UpdateAnimation` segun `grounded`, `moving` y `vel.y`.
- **Espada:** el arma flota a 18 px del jugador (`AnchorWorld`). Mantener F (>= 0.15 s) llama a `weapon::TryHeavyAttack()`,
  que salta directo al 3er golpe del combo (2 de daño) y lo repite mientras se siga manteniendo.
- **Vida de los enemigos:** `enemy::health` / `TakeHit(damage)`. El slime (`enemyLinear`) tiene 2 de vida; los demas, 1.
  Daño: golpe comun de espada = 1, golpe final del combo = 2, proyectil magico = 1. Al recibir un golpe sin morir parpadea en rojo.
- **Proyectil magico:** 64 px (antes 32) para alcanzar a los slimes que caminan por el piso.

## Ultimos cambios: progreso y jefe final

- **Borrar progreso** (selector de niveles, boton rojo "Borrar progreso y empezar de cero"): abre una ventana de
  confirmacion que avisa que se pierde el progreso y que no se puede deshacer. "Cancelar" o ESC no tocan nada.
  "Si, borrar y empezar" llama a `saveManager::ResetProgress()` (vuelve a los valores iniciales con `SetDefaults()` y
  elimina `save.dat`) y arranca el nivel 1 sin arma. Los volumenes viven en `scene`, no en `saveManager`, asi que no se tocan.
- **Onda del jefe final**: la "onda expansiva" ya no es un proyectil unico de 50x760 px (imposible de esquivar). Ahora es una
  pared de 8 segmentos independientes (`waveSegment`, 60x95 px) que cruza la arena a 260 px/s; la maneja `boss`
  (`SpawnWave`, `UpdateWaves`, `DrawWaves`). Cada segmento tiene su propia colision:
  - un tajo de espada (`SWORD_HIT`, cuya zona de dano mide al menos la altura del jugador) o una bola de fuego del baston
    rompen solo los segmentos que tocan (`boss::BreakWave`); como un golpe mide mas de 95 px siempre abre un hueco de
    190 px o mas (el personaje mide 128);
  - un segmento roto pierde la colision en el mismo instante (queda solo un destello de 0,3 s);
  - los segmentos vivos lastiman al jugador (`boss::WaveTouches`); el que lo golpea se rompe, asi un paso cuenta un golpe;
  - la onda se elimina al salir de la pantalla, al romperse entera, al morir el jefe o al reiniciar el nivel.

## Paredes, vision de los enemigos y disparos enemigos

- `Level` le pasa a los enemigos un `enemyWorld` (posicion y centro del jugador + las paredes = puertas cerradas).
- **No atraviesan paredes:** `enemy::ResolveWalls` frena al perro contra la puerta y hace que el slime y el orco, al patrullar,
  se den la vuelta. La abeja (orbita fija) no se ve afectada.
- **No ven a traves de ellas:** `enemy::CanSeePlayer` comprueba que el segmento entre los centros del enemigo y del jugador
  no cruce ninguna pared (recorte de Liang-Barsky). El perro solo persigue si ve al jugador (si no, frena y espera) y el orco
  solo dispara si lo ve. Las plataformas finas no tapan la vista.
- **Disparos enemigos** (tiros dirigidos y rafagas del jefe, magia de los orcos): en `Level::UpdateGameplay` se destruyen al
  tocar el piso, las plataformas (fijas y moviles) o una puerta cerrada. La pared de fuego del jefe no se ve afectada: sus
  segmentos se rompen solo con el arma.
