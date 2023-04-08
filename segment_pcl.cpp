#include <iostream>
#include <vector>
#include <pcl/filters/crop_box.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/common/common.h>
#include <pcl/impl/point_types.hpp>

#include "csv_io.h"

// tl = top left
pcl::PointXYZ local_map_tl;
pcl::PointXYZ local_map_tr;
pcl::PointXYZ local_map_bl;
pcl::PointXYZ local_map_br;
int local_map_dim = 2; // number of grid cells per map side length

void translate_rover_to_local(pcl::PointCloud<pcl::PointXYZ> &rover_cloud, pcl::PointXYZ local_origin);
void segment_pcl(const pcl::PointCloud<pcl::PointXYZ>::Ptr rover_cloud);


struct PointT {
    PCL_ADD_POINT4D; // adds x, y, z coordinates
    float intensity;
    uint8_t r, g, b;
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW // ensures proper memory alignment
} EIGEN_ALIGN16;

int main(void)
{
    using namespace pcl;
    // pcl::PointCloud<pcl::PointXYZ>::Ptr pcd(new pcl::PointCloud<pcl::PointXYZ>);

    // if (pcl::io::loadPCDFile<pcl::PointXYZ> ("p213.pcd", *pcd) == -1) //* load the file
    // {
    //     PCL_ERROR("Couldn't read file p213.pcd \n");
    //     return (-1);
    // }
    // std::cout << "Loaded "
    //           << pcd->width * pcd->height
    //           << " data points " << std::endl;

    // Create the point cloud
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

    // Fill in the cloud data with randomly generated set of 15 points
    cloud->width = 15;
    cloud->height = 1;
    cloud->points.resize(cloud->width * cloud->height);

    // Generate the data
    srand(time(0));
    for (auto &point : *cloud)
    {
        point.x = 1024 * rand() % 9;
        point.y = 1024 * rand() % 9;
        point.z = 1024 * rand() % 9;
    }

    std::cout << "Point cloud data: " << cloud->size() << " points" << std::endl;
    for (const auto &point : *cloud)
    {
        std::cout << "    " << point.x << " "
                  << point.y << " "
                  << point.z << std::endl;
    }

    segment_pcl(cloud);
    return 0;
}
void segment_pcl(const pcl::PointCloud<pcl::PointXYZ>::Ptr rover_cloud)
{
    using namespace pcl;

    // set local map bounds
    local_map_tl = pcl::PointXYZ(-8, 8, 0);
    // std::cout << local_map_tl.x << std::endl;
    local_map_tr = pcl::PointXYZ(8, 8, 0);
    local_map_bl = pcl::PointXYZ(-8, -8, 0);
    local_map_br = pcl::PointXYZ(8, -8, 0);

    //get min_max x and y of input cloud to segment
    pcl::PointXYZ min;
    pcl::PointXYZ max;
    pcl::getMinMax3D (*rover_cloud, min, max);
    double x_sz = max.x - min.x;
    double y_sz = max.y - min.y;
    double x_grid_sz = x_sz / local_map_dim;
    double y_grid_sz = y_sz / local_map_dim;

    std::cout << min.x << " " << min.y << "\n";
    std::cout << max.x << " " << max.y << "\n\n";

    // length of local map sides (is a square)
    double local_map_len = local_map_tr.x - local_map_tl.x;
    double cell_len = local_map_len / local_map_dim;
    //std::cout << cell_len << std::endl;

    // // Translate crop box up by 1
    // cropBoxFilter.setTranslation(Eigen::Vector3f(0, 1, 0));

    for (int i = 0; i < local_map_dim; i++)
    {
        for (int j = 0; j < local_map_dim; j++)
        {
            // std::cout << "Min point: X : " << local_map_tl.x + (cell_len * j) << " Y : " << local_map_bl.y + (cell_len * i);
            // std::cout << "      Max point: X : " << local_map_tl.x + (cell_len * (j + 1)) << " Y : " << local_map_bl.y + (cell_len * (i + 1));
            // std::cout << std::endl;

            CropBox<PointXYZ> cropBoxFilter (true);
            cropBoxFilter.setInputCloud (rover_cloud);
            //order is bottom left --> top right
            // Eigen::Vector4f min_pt (local_map_tl.x + (cell_len * j), local_map_bl.y + (cell_len * i), -1000.0f, -1000.0f);
            // Eigen::Vector4f max_pt (local_map_tl.x + (cell_len * (j + 1)), local_map_bl.y + (cell_len * (i + 1)), 1000.0f, 1000.0f);

            Eigen::Vector4f min_pt (min.x + (x_grid_sz * j), min.y + (y_grid_sz * i), -1000.0f, -1000.0f);
            Eigen::Vector4f max_pt (min.x + (x_grid_sz * (j + 1)), min.y + (y_grid_sz * (i + 1)), 1000.0f, 1000.0f);

            cropBoxFilter.setMin (min_pt);
            cropBoxFilter.setMax (max_pt);

            // Cloud
            PointCloud<PointXYZ> cloud_out;
            cropBoxFilter.filter (cloud_out);
            std::cout << cloud_out.width << std::endl;

            for (const auto& point: cloud_out)
            {
                std::cout << i << "th Row, " << j << "th Col: " << point.x << " "
                          << point.y << " "
                          << std::endl;
            }
            // do some traversibility stuff here...
        }
    }
}
//translates all points to local map frame
void translate_rover_to_local(pcl::PointCloud<pcl::PointXYZ> &rover_cloud, pcl::PointXYZ local_origin)
{
    for (auto &point : rover_cloud)
    {
        point.x = point.x - local_origin.x;
        point.y = point.y - local_origin.y;
    }
}