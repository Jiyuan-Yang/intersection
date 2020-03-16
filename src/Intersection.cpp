#include "Intersection.h"

Intersection::Intersection() {}

int Intersection::getAllPoints() {
	int n, i;
	double x1, x2, y1, y2, radius;
	char type;
	cin >> n;
	for (i = 0; i < n; i++) {
		cin >> type;
		if (isLine(type)) {
			cin >> x1 >> y1 >> x2 >> y2;
			points.push_back(Point(x1, y1, type));
			vectors.push_back(Point(x2 - x1, y2 - y1, type)); // whole Length
		}
		else if (isCircle(type)) {
			cin >> x1 >> y1 >> radius;
			circles.push_back(Circle(Heart(x1, y1, type), radius));
			// Radius.push_back(radius);
		}
		else {
			fprintf(stderr, "Type error detected in line %d: type:%c\n", n, type);
			return -1;
		}
	}
	return 0;
}

void Intersection::solveLineLineIntersection() {
	int i, j, n;
	Vector u, v, w;
	n = (int)vectors.size();
	for (i = 0; i < n; i++) {
		for (j = i + 1; j < n; j++) {
			u = points[i] - points[j];
			v = vectors[i];
			w = vectors[j];
			double denominator = Cross(v, w);
			if (dcmp(denominator) == 0) { // parallel case
				continue;
			}
			double t_i = 1.0 * Cross(w, u) / denominator;
			double t_j = 1.0 * Cross(v, u) / denominator;

			char type_i = points[i].type;
			char type_j = points[j].type;

			if (LineIsValid(type_i, t_i) && LineIsValid(type_j, t_j)) { // only when double valid is valid
				intersects.push_back(points[i] + v * t_i);
				// intersects.push_back(points[j] + w * t_j); // the same, only for validate t_j
			}
		}
	}
}

void Intersection::solveLineCircleIntersection() {
	int i, j;
	for (i = 0; i < (int)vectors.size(); i++) {
		for (j = 0; j < (int)circles.size(); j++) {
			Point p = points[i];
			Vector pq = vectors[i];
			Circle c = circles[j];
			double radius = c.radius;
			char type = p.type;
			Vector norm_pq = Norm(pq);
			Heart r = c.center;
			Vector pr = r - p;
			Vector norm_pr = Norm(pr);
			Point q = p + pq;

			double RM = CalculateRM(c, p, pq);
			double PR = Length(pr);
			int status = dcmp(RM - radius);
			if (status == 0) { // tangency
				double PM = CalculatePM(c, p, pq);
				double t = PM / Length(pq);
				if (LineIsValid(type, t)) {
					Point m = p + norm_pq * PM;
					intersects.push_back(m);
				}

				// For validation
				double PS = CalculatePS_l(c, p, pq);
				// assert(dcmp(fabs(PS) - fabs(PM)) == 0);
			}
			else if (status > 0) { // disjoint
				continue;
			}
			else { // intersect
				if (dcmp(RM) == 0 && dcmp(PR) != 0) {

					double PS_l = Length(pr) + radius;
					double t = PS_l / Length(pq);
					if ((p < r && r < q) || (r < p && q < r)) {
						t = -t;
					}
					if (LineIsValid(type, t)) {
						Point s_l = p + norm_pr * PS_l;
						intersects.push_back(s_l);
					}

					double PS_r = 2 * radius - PS_l;
					t = PS_r / Length(pq);
					if ((p < r && r < q) || (r < p && q < r)) {
						t = -t;
					}
					if (LineIsValid(type, t)) {
						Point s_r = p + norm_pr * PS_r;
						intersects.push_back(s_r);
					}
				}
				else {
					double PM = CalculatePM(c, p, pq);
					double PS_l = CalculatePS_l(c, p, pq);
					double t = PS_l / Length(pq);
					if (LineIsValid(type, t)) {
						Point s_l = p + norm_pq * PS_l;
						intersects.push_back(s_l);
					}

					double PS_r = 2 * PM - PS_l;
					t = PS_r / Length(pq);
					if (LineIsValid(type, t)) {
						Point s_r = p + norm_pq * PS_r;
						intersects.push_back(s_r);
					}
				}
			}
		}
	}
}

void Intersection::solveCircleCircleIntersection() {
	int i, j;
	for (i = 0; i < (int)circles.size(); i++) {
		for (j = i + 1; j < (int)circles.size(); j++) {
			Heart p = circles[i].center;
			Heart q = circles[j].center;
			Vector pq = q - p;
			Vector norm_pq = Norm(pq);
			double p2q = Length(pq);
			double radius_1 = circles[i].radius;
			double radius_2 = circles[j].radius;

			int status = dcmp(p2q - radius_1 - radius_2);
			if (status == 0) { // outside tangency
				intersects.push_back(p + norm_pq * radius_1);
			}
			else if (status > 0) { // outside disjoint
				continue;
			}
			else {
				status = dcmp(fabs(radius_1 - radius_2) - p2q);
				if (status == 0) { // inside tangency
					intersects.push_back(p + norm_pq * radius_1);
				}
				else if (status > 0) { // inside disjoint
					continue;
				}
				else {
					// double rad = CalculateAngle(circles[i], circles[j]);
					double ratio = (radius_1 * radius_1 + p2q * p2q - radius_2 * radius_2) / (2 * radius_1 * p2q);
					if (ratio > 1 - eps) {
						ratio = 1 - eps;
					}
					else if (ratio < eps - 1) {
						ratio = eps - 1;
					}
					double rad = acos(ratio);
					Vector s_l = Rotate(norm_pq, rad) * radius_1;
					Vector s_r = Rotate(norm_pq, -rad) * radius_1;
					intersects.push_back(p + s_l);
					intersects.push_back(p + s_r);
				}
			}
		}
	}
}

int Intersection::solveIntersection() {
	if (points.size() > 1) {
		solveLineLineIntersection();
	}
	if (circles.size() > 1) {
		solveCircleCircleIntersection();
	}
	if (points.size() > 0 && circles.size() > 0) {
		solveLineCircleIntersection();
	}

	// ------------------ Delete Duplicates ------------------------

	int size = (int)intersects.size();

	if (isPrintAllIntersections) {
		cout << "----------------------------------------------------------------------" << endl;
		cout << "before unique: " << size << endl;
		printAllIntersections();
	}

	sort(intersects.begin(), intersects.end());
	auto new_end = unique(intersects.begin(), intersects.end());

	assert(size == intersects.size()); // Fake Delete Duplicates

	intersects.erase(new_end, intersects.end()); // Real Delete Duplicates

	if (isPrintAllIntersections) {
		cout << "----------------------------------------------------------------------" << endl;
		cout << "after unique: " << intersects.size() << endl;
		printAllIntersections();
	}

	return (int)intersects.size();
}


void Intersection::printAllPoints(vector<Point> v) {
	auto it = v.begin();
	while (it != v.end()) {
		printf("%.10lf %.10lf\n", it->x, it->y);
		it++;
	}
}

void Intersection::printAllIntersections() {
	printAllPoints(intersects);
}