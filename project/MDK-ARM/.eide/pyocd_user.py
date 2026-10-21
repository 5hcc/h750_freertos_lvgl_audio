from pyocd.core.memory_map import FlashRegion


def will_connect(board):
    board.target.memory_map.add_region(
        FlashRegion(
            start=0x90000000,
            end=0x907FFFFF,
            name="external_qspi",
            flm=r"D:\KEIL C51\ARM\Flash\ATK-DNH750_QSPI_W25Q64JV.flm",
        )
    )