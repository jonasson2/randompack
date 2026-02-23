Wheels commands:
----------------
.yml files are in <root>/.github/workflows, but following can be run from python/
gh workflow list
gh workflow run xxx.yml        # trigger wheel building
gh run watch
gh run list
gh run view <RUN-ID> --log-failed
gh run list --json databaseId --jq '.[].databaseId'  # just the id-s, use in foreach...
gh run delete <RUN-ID>
– or view on github.com–Actions

rm -rf wheelhouse/*
gh run download <RUN-ID> -D wheelhouse

And more gh commands:
---------------------
gh release create v0.1.0 --title "v0.1.0" --notes "Initial release."
(also triggers wheel building)
              
Upload to TestPyPI:
-------------------
cd <root>/python
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
