#!/usr/bin/env python
from __future__ import annotations
from flask import Flask, send_file
from flask_restful import Api
from os.path import join

app = Flask(__name__)
api = Api(app)

@app.route("/api/files/<string:filename>")
def return_file(filename):
    # TODO: Extract file path routing
    return send_file(join("data", filename))

def main() -> int:
    app.run(debug=True)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())

