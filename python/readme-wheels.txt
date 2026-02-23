Wheels commands:
----------------
gh workflow list
gh workflow run xxx.yml        # trigger wheel building
gh run list
gh run watch
gh run list --json databaseId --jq '.[].databaseId'
gh run download <RUN_ID> -D wheelhouse
– or view on github.com–Actions

And more gh commands:
---------------------
gh release create v0.1.0 --title "v0.1.0" --notes "Initial release."
<also triggers wheel building>

Prepare upload to PyPi:
-----------------------
Get rid of conda:
conda deactivate [maybe repeat]
which python
which pip

Create a virtual environment in ~/venvs with python, pip and twine:
-------------------------------------------------------------------
python -m venv ~/venvs/release
source ~/venvs/release/bin/activate
pip install -U build twine
which python
which pip

Then from the project root:
---------------------------
cd python
python -m build -s  # build an sdist (<project>.tar.gz) in dist/
gh run download <run-id> -n wheels-ubuntu-latest -D dist (or copy from wheelhouse
gh run download <run-id> -n wheels-macos-latest  -D dist
gh run download <run-id> -n wheels-windows-latest -D dist
python -m twine check dist/*
python -m twine upload --repository testpypi dist/*  # test upload

Test upload:
------------
python -m venv /tmp/rp-test
source /tmp/rp-test/bin/activate
pip install -i https://test.pypi.org/simple randompack
python -c "import randompack; print(randompack.Rng())"

Real upload:
-----------
python -m twine upload dist/*

To build and load the library locally:
--------------------------------------
cd python
pip install -e .
python
>>> import randompack
