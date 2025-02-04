<h1 align="center">pymaeparser</h1>

<p align="center">Read and write MAE files.</p>

<p align="center">
  <a href="https://github.com/SimonBoothroyd/mdtop/actions?query=workflow%3Aci">
    <img alt="ci" src="https://github.com/SimonBoothroyd/mdtop/actions/workflows/ci.yaml/badge.svg" />
  </a>
  <a href="https://opensource.org/licenses/MIT">
    <img alt="license" src="https://img.shields.io/badge/License-MIT-yellow.svg" />
  </a>
</p>

---

This package provides a simple set of python bindings for reading and writing Maestro MAE files using the
[maeparser](https://github.com/schrodinger/maeparser) library provided by Schrodinger.

## Installation

This package can be installed using `conda` (or `mamba`, a faster version of `conda`):

```shell
mamba install -c conda-forge pymaeparser
```

## Example

MAE files are read into a list of dictionaries, where each dictionary represents a structure. The structure dictionary
has the following entries:

- `title`: The title of the structure.
- `props`: A dictionary of properties.
- `atoms`: A dictionary of atom data.
- `bonds`: A dictionary of bond data.

To read a MAE file:

```python
import pymaeparser

structures = pymaeparser.read_mae("tests/data/benzoate.mae")
```

MAE files can also be written from a list of structure dictionaries:

```python
import pymaeparser

structure = {
    'title': 'methane',
    'props': {},
    'atoms': {
        'i_m_atomic_number': [6, 1, 1, 1, 1],
        'i_m_formal_charge': [0, 0, 0, 0, 0],
        'i_m_mmod_type': [3, 48, 48, 48, 48],
        'i_m_residue_number': [0, 0, 0, 0, 0],
        'i_pdb_PDB_serial': [1, 2, 3, 4, 5],
        'r_m_x_coord': [-0.022, -0.666, -0.378, 0.096, 0.97],
        'r_m_y_coord': [0.003, 0.888, -0.858, -0.315, 0.281],
        'r_m_z_coord': [0.016, -0.101, -0.588, 1.064, -0.391],
        's_m_chain_name': [' ', ' ', ' ', ' ', ' '],
        's_m_pdb_atom_name': [' C  ', ' H  ', ' H  ', ' H  ', ' H  '],
        's_m_pdb_residue_name': ['UNK ', 'UNK ', 'UNK ', 'UNK ', 'UNK '],
    },
    'bonds': {
        'i_m_from': [1, 1, 1, 1],
        'i_m_from_rep': [2, 2, 2, 2],
        'i_m_order': [1, 1, 1, 1],
        'i_m_to': [2, 3, 4, 5],
        'i_m_to_rep': [2, 2, 2, 2]
    }
}
pymaeparser.write_mae([structure], "output.mae")
```
