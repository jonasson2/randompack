Wheels commands:
----------------
.yml files are in <root>/.github/workflows, but following can be run from python/
gh workflow list
gh workflow run xxx.yml        # trigger wheel building
gh run watch
gh run list
gh run list --json databaseId --jq '.[].databaseId'  # just the id-s, use in foreach...
gh run download <RUN_ID> -D wheelhouse
gh run delete <RUN_ID>
– or view on github.com–Actions

And more gh commands:
---------------------
gh release create v0.1.0 --title "v0.1.0" --notes "Initial release."
(also triggers wheel building)

The following may be unnecessary:
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
cp wheelhouse/*/*.whl dist
python -m twine check dist/*
python -m twine upload --repository testpypi dist/*  # test upload

Try out what was uploaded:
--------------------------
python -m venv /tmp/rp-test
source /tmp/rp-test/bin/activate
pip install \
  --index-url https://test.pypi.org/simple \
  --extra-index-url https://pypi.org/simple \
  randompack
python -c "import randompack; print(randompack.Rng())"

Real upload:
------------
python -m twine upload dist/*

To build and load the library locally:
--------------------------------------
cd python
pip install -e .
python
>>> import randompack
