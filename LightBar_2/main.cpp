#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main()
{
	VideoCapture cap("D:\\video\\lights.mp4");
	Mat frame;
	while (true)
	{
		cap.read(frame);
		if (frame.empty())
		{
			break;
		}

		imshow("frame", frame);


		//��ֵ��
		Mat GRAY, BI;
		cvtColor(frame, GRAY, COLOR_BGR2GRAY);
		threshold(GRAY, BI, 150, 256, THRESH_BINARY);
		Mat mask = BI.clone();


		//��̬ѧ����ȥ������
		Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
		morphologyEx(mask, mask, MORPH_OPEN, kernel, Point(-1, -1), 1);
		morphologyEx(mask, mask, MORPH_CLOSE, kernel, Point(-1, -1), 2);


		// ��������
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(mask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);


		// ɸѡ��������
		vector<RotatedRect> light_bars;
		const float MIN_AREA = 150.0;        // ��С�������
		const float MAX_AREA = 200.0;        // ����������
		const float MIN_ASPECT_RATIO = 2.5;  // ��С�����
		const float MAX_ASPECT_RATIO = 7.0;  // ��󳤿��

		for (const auto& contour : contours)
		{
			// ���ɸѡ
			double area = contourArea(contour);
			if (area < MIN_AREA && area>MAX_AREA)
			{
				continue;
			}

			RotatedRect rect = minAreaRect(contour);

			// �����
			float width = rect.size.width;
			float height = rect.size.height;
			float aspect_ratio = max(width, height) / min(width, height);

			if (aspect_ratio >= MIN_ASPECT_RATIO && aspect_ratio <= MAX_ASPECT_RATIO && width < 50.0 && height>20.0)
			{
				light_bars.emplace_back(rect);
			}
		}
		

		// ���Ƶ����߽��
		for (const auto& bar : light_bars)
		{
			Point2f vertices[4];
			bar.points(vertices);
			for (int i = 0; i < 4; i++)
			{
				line(frame, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0), 2);
			}
		}


		//�װ��������
		if (light_bars.size() >= 2)
		{
			const float MAX_ANGLE_DIFF = 10.0f; // ���ǶȲ�
			pair<RotatedRect, RotatedRect> best_pair;
			float max_distance = 0;

			for (size_t i = 0; i < light_bars.size(); ++i)
			{
				for (size_t j = i + 1; j < light_bars.size(); ++j)
				{
					// �ǶȲ�
					float diff = abs(light_bars[i].angle - light_bars[j].angle);

					if (diff <= MAX_ANGLE_DIFF)
					{
						// ���ĵ����
						float distance = norm(light_bars[i].center - light_bars[j].center);

						if (distance > max_distance)
						{
							max_distance = distance;
							best_pair = { light_bars[i], light_bars[j] };

						}
					}
				}
			}


			// ���Ƽװ�����
			if (max_distance > 0)
			{
				vector<Point> combined_points;
				Point2f vertices_1[4];

				// �ռ������Ķ���
				best_pair.first.points(vertices_1);
				for (int i = 0; i < 4; ++i)
				{
					combined_points.push_back(Point(vertices_1[i]));
				}
				best_pair.second.points(vertices_1);
				for (int i = 0; i < 4; ++i)
				{
					combined_points.push_back(Point(vertices_1[i]));
				}

				// ������Ӿ���
				//Rect deck_area = boundingRect(combined_points);
				//rectangle(frame, deck_area, Scalar(0, 0, 255), 3);

				RotatedRect deck_area = minAreaRect(combined_points);
				Point2f vertices_2[4];
				deck_area.points(vertices_2);
				for (int i = 0; i < 4; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						line(frame, vertices_2[j], vertices_2[(j + 1) % 4], Scalar(0, 0, 255), 2, LINE_AA);
					}
				}
			}


			imshow("result", frame);


			int c = cv::waitKey(10);
			if (c == 27)
			{
				break;
			}

		}
	}
	waitKey(0);
	return 0;
}