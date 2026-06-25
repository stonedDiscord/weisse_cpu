# Die Weiße CPU

Dies ist ein Reverse-Engineering Projekt fuer die "Elektronische Steuereinheit" von adp.

```mermaid
graph LR
    4087-000101

    subgraph CPU_BLK["CPU"]
        C["8085A\nICB2\n74LS373 addr latch\n74LS245 data buf"]
    end

    subgraph ADEC["Address Decode"]
        D["74LS138 (ICH5)\nmem bank select\n\n74LS42 ×3\nI/O port select"]
    end

    subgraph MMAP["Memory Map"]
        M1["0x0000–0x3FFF\nSocket 1 · ICD6 27128  (16 KB)"]
        M2["0x4000–0x7FFF\nSocket 2 · ICE6 27128  (16 KB)"]
        M3["0x8000–0x8FFF\nSocket 3 · ICC5 2764  (8 KB, empty)"]
        M4["0x9000–0x900F\nRTC · ICA3 HD146818  (16 B, mem-mapped)"]
        M5["0xC000–0xFFFF\nSRAM · ICC6 CY62256  (32 KB chip)"]
    end

    subgraph IOMAP["I/O Map"]
        I1["0x50  8279 data\n0x51  8279 cmd"]
        I2["0x60–6F  8256 regs"]
        I3["0x70  coin ejector\n0x71  sound start\n0x72  tone note\n0x73  PPI ctrl"]
    end

    subgraph EXT["External"]
        E1["16× 7-seg display\nLamp matrix 8×8\nButton/coin matrix 8×8"]
        E2["Serial 4800 8N1\nreel stepper motor\nreel optical sensor"]
        E3["Coin ejector\nTone generator"]
    end

    C <--> D
    D <--> MMAP
    D <--> IOMAP
    I1 --- E1
    I2 --- E2
    I3 --- E3
```

Bei 4040 Platinen sind die IO Adressen anders verteilt:

| IO | Ports |
| ---- | ----------------|
| 0x70-0x73 | 8255: Auswerfer+Zähler+Sound |
| 0x80-0x81 | 8279: Lampen+Tasten |
| 0x90-0x9f | 8256: Timer+Seriell+Walzen |

Aus diesem Grund sind die Programme nicht mit beiden Platinen kompatibel.
Die Lampen und Timer würden ohne Anpassung nicht funktionieren.
