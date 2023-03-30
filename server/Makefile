RM     = rm -rf

MAIN   = main.py
REQS   = requirements.txt
VENV   = .venv

PYTHON = $(VENV)/bin/python3
PIP    = $(VENV)/bin/pip

run: $(VENV)/bin/activate
	$(PYTHON) $(MAIN)

$(VENV)/bin/activate: $(REQS)
	python3 -m venv $(VENV)
	$(PIP) install -r $<

clean:
	$(RM) __pycache__
	$(RM) $(VENV)

.PHONY: run clean
