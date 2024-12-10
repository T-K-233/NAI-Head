from setuptools import find_packages, setup

setup(
    name="vtubestudiobridge",
    version="0.1.0",
    author="-T.K.-",
    author_email="t_k_233@outlook.com",
    description="A python library for interacting with the VTube Studio API without asyncio",
    url="https://github.com/T-K-233/VTubeStudioBridge",
    license="MIT",
    python_requires=">=3.8",
    install_requires=[
        "websockets",
    ],
    package_dir={"": "src"},
    packages=find_packages(where="src"),
)