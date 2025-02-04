import pathlib

import pytest

import pymaeparser


@pytest.fixture
def data_dir() -> pathlib.Path:
    return pathlib.Path(__file__).parent / "data"


def test_pymaeparser(data_dir, tmp_path):
    parsed = pymaeparser.read_mae(data_dir / "benzoate.mae")

    pymaeparser.write_mae(parsed, tmp_path / "benzoate-new.mae")
    reparsed = pymaeparser.read_mae(tmp_path / "benzoate-new.mae")

    assert parsed == reparsed
    assert isinstance(reparsed[0]["atoms"]["b_m_prop_a"][0], bool)
    assert isinstance(reparsed[0]["props"]["b_m_prop_d"], bool)
