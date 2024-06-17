#pragma once
template <typename T>
class Geometry
{
public:
	Geometry() {}
	~Geometry() {}

	void SetVertices(const vector<T>& vertices) { _vertices = vertices; }
	void SetIndices(const vector<uint32>& indices) { _indices = indices; }
	void AddVertex(const T& vertex) { _vertices.push_back(vertex); }
	void AddVertices(const vector<T>& vertices) { _vertices.insert(_vertices.end(), vertices.begin(), vertices.end()); }

	void AddIndex(uint32 index) { _indices.push_back(index); }
	void AddIndices(const vector<uint32>& indices) { _indices.insert(_indices.end(), indices.begin(), indices.end()); }
	vector<T>& GetVertices() { return _vertices; }
	vector<uint32>& GetIndices() { return _indices; }
private:
	vector<T> _vertices;
	vector<uint32> _indices;
};

