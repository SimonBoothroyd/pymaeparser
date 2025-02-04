"""Read and write MAE files using the maeparser library."""

import pathlib
import typing


def read_mae(path: str | pathlib.Path) -> list[dict[str, typing.Any]]:
    """Read an MAE file and return a dictionary with the parsed data.

    Args:
        path: The path to the MAE or GZipped MAE file.

    Returns:
        A list of data for each structure in the MAE file. Each structure is a
        dictionary with the following keys:

            - `title`: The title of the structure.
            - `atoms`: A list of atoms in the structure.
            - `bonds`: A list of bonds in the structure.
            - `props`: A dictionary of top level properties of the structure.
    """
    from .pymaeparser_ext import read_mae as read_mae_ext

    structures = read_mae_ext(str(path))

    for structure in structures:
        if "title" not in structure:
            structure["title"] = None
        if "atoms" not in structure:
            structure["atoms"] = {}
        if "bonds" not in structure:
            structure["bonds"] = {}
        if "props" not in structure:
            structure["props"] = {}

    return structures


def write_mae(
    structures: list[dict[str, typing.Any]], path: str | pathlib.Path
) -> dict[str, typing.Any]:
    """Write a dictionary with the structure data to an MAE file.

    The structure dictionary should have the following keys:
        - `title`: Title of the structure.
        - `atoms`: Atoms in the structure.
        - `bonds`: Bonds in the structure.
        - `props`: Top level properties of the structure.

    Each value of `atoms` and `bonds` should be a dictionary of lists, with keys
    corresponding to property names, and values corresponding to the property values.
    All lists must have the same length.

    `props` should be a dictionary values, rather than lists.
    """
    from .pymaeparser_ext import write_mae as write_mae_ext

    for structure in structures:
        found_keys = {*structure}
        allowed_keys = {"title", "atoms", "bonds", "props"}

        if len(found_keys - allowed_keys) > 0:
            raise ValueError(
                f"Unexpected keys in structure: {found_keys - allowed_keys}"
            )

    return write_mae_ext(structures, str(path))


__all__ = ["read_mae", "write_mae"]
