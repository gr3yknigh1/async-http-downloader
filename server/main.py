#!/usr/bin/env python
from __future__ import annotations
from flask import Flask
from flask_restful import Api

app = Flask(__name__)
api = Api(app)

def main() -> int:
    app.run(debug=True)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())

